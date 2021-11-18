#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
class PriorityCollection {
public:
    using Id = int;

    Id Add(T object) {
        Id id = elements.size();

        data.insert({0, id});
        elements.push_back({move(object), 0});

        return id;
    }

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end, IdOutputIt ids_begin) {
        transform(range_begin, range_end, ids_begin, [this](T& elem) {
            return Add(move(elem));
        });
    }

    bool IsValid(Id id) const {
        return id >= 0 && id < elements.size() && elements[id].second != NONE_PRIORITY;
    }

    const T& Get(Id id) const {
        return elements.at(id).first;
    }

    void Promote(Id id) {
        data.erase({elements[id].second, id});
        data.insert({++elements[id].second, id});
    }

    pair<const T&, int> GetMax() const {
        Id id = data.rbegin()->second;
        return elements.at(id);
    }

    pair<T, int> PopMax() {
        Id id = data.rbegin()->second;

        int priority = elements[id].second;

        data.erase({priority, id});
        elements[id].second = NONE_PRIORITY;

        return {move(elements[id].first), priority};
    }

private:
    set<pair<int, Id>> data;
    vector<pair<T, int>> elements;

    const int NONE_PRIORITY = -1;
};


class StringNonCopyable : public string {
public:
    using string::string;
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }
    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);
    return 0;
}