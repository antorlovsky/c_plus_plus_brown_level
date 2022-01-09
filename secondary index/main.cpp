#include "test_runner.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <tuple>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

bool operator<(const Record& lhs, const Record& rhs) {
    return tie(lhs.id, lhs.title, lhs.user, lhs.timestamp, lhs.karma)
        < tie(rhs.id, rhs.title, rhs.user, rhs.timestamp, rhs.karma);
}

class Database {
public:
    bool Put(const Record& record) {
        auto [it, success] = storage.insert({record.id, {record, {}, {}, {}}});

        if (!success)
            return false;

        auto& data = it->second;
        const Record* record_ptr = &data.record;

        data.timestamp_iter = timestamp_index.insert({record.timestamp, record_ptr});
        data.karma_iter = karma_index.insert({record.karma, record_ptr});
        data.user_iter = user_index.insert({record.user, record_ptr});

        return true;
    }

    const Record* GetById(const string& id) const {
        if (storage.count(id) == 0)
            return nullptr;

        return &storage.at(id).record;
    }

    bool Erase(const string& id) {
        auto it = storage.find(id);

        if (it == storage.end())
            return false;

        timestamp_index.erase(it->second.timestamp_iter);
        karma_index.erase(it->second.karma_iter);
        user_index.erase(it->second.user_iter);

        storage.erase(it);

        return true;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto start_it = timestamp_index.lower_bound(low);
        auto end_it = timestamp_index.upper_bound(high);

        while (start_it != timestamp_index.end() && start_it != end_it) {
            if (!callback(*start_it->second))
                break;

            ++start_it;
        }
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto start_it = karma_index.lower_bound(low);
        auto end_it = karma_index.upper_bound(high);

        while (start_it != karma_index.end() && start_it != end_it) {
            if (!callback(*start_it->second))
                break;

            ++start_it;
        }
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto [start_it, end_it] = user_index.equal_range(user);

        while (start_it != user_index.end() && start_it != end_it) {
            if (!callback(*start_it->second))
                break;

            ++start_it;
        }
    }

private:
    template <typename Type>
    using Index = multimap<Type, const Record*>;

    struct Data {
        Record record;
        Index<int>::iterator timestamp_iter;
        Index<int>::iterator karma_iter;
        Index<string>::iterator user_iter;
    };

    using Id = string;

    unordered_map<Id, Data> storage;
    Index<int> timestamp_index;
    Index<int> karma_index;
    Index<string> user_index;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
    db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
    db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
    db.Erase("id");
    db.Put({"id", final_body, "not-master", 1536107260, -10});

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}