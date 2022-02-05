#include "Common.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>

using namespace std;

class LruCache : public ICache {
public:
    LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings& settings)
    : settings_(settings), books_unpacker_(move(books_unpacker)), current_size(0), max_rank(0) {}

    BookPtr GetBook(const string& book_name) override {
        lock_guard lg = lock_guard(mutex_);
        auto it = books_.find(book_name);

        BookPtr book;

        if (it == books_.end()) {
            book = books_unpacker_->UnpackBook(book_name);

            while (!books_.empty() && current_size + book->GetContent().size() > settings_.max_memory) {
                auto min_it = min_element(books_.begin(), books_.end(), [](const auto& lhs, const auto& rhs) {
                    return lhs.second.second < rhs.second.second;
                });

                current_size -= min_it->second.first->GetContent().size();
                books_.erase(min_it);
            }

            if (current_size + book->GetContent().size() <= settings_.max_memory) {
                books_[book_name] = {book, ++max_rank};
                current_size += book->GetContent().size();
            }
        }
        else {
            it->second.second = ++max_rank;
            book = it->second.first;
        }

        return book;
    }

private:
    shared_ptr<IBooksUnpacker> books_unpacker_;
    Settings settings_;
    size_t current_size;
    int max_rank;
    unordered_map<string, pair<BookPtr, int>> books_;
    mutex mutex_;
};


unique_ptr<ICache> MakeCache(shared_ptr<IBooksUnpacker> books_unpacker, const ICache::Settings& settings) {
    return make_unique<LruCache>(move(books_unpacker), settings);
}