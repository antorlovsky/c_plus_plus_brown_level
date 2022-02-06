#include "test_runner.h"

#include <cstddef>
#include <utility>

using namespace std;

template <typename T>
class UniquePtr {
private:
    T* object;

    void DeleteObject() {
        if (object)
            delete object;
    }

public:
    UniquePtr() : object(nullptr) {}

    UniquePtr(T* ptr) : object(ptr) {}

    UniquePtr(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) : object(other.Release()) {}

    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr& operator=(nullptr_t) {
        DeleteObject();
        object = nullptr;

        return *this;
    }

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) {
            DeleteObject();
            object = other.Release();
        }

        return *this;
    }

    ~UniquePtr() {
        DeleteObject();
    }

    T& operator*() const {
        return *object;
    }

    T* operator->() const {
        return object;
    }

    T* Release() {
        T* obj = object;
        object = nullptr;

        return move(obj);
    }

    void Reset(T* ptr) {
        DeleteObject();
        object = ptr;
    }

    void Swap(UniquePtr& other) {
        T* obj = other.Release();
        other.Reset(object);
        object = obj;
    }

    T* Get() const {
        return object;
    }
};


struct Item {
    static int counter;
    int value;
    Item(int v = 0): value(v) {
        ++counter;
    }
    Item(const Item& other): value(other.value) {
        ++counter;
    }
    ~Item() {
        --counter;
    }
};

int Item::counter = 0;


void TestLifetime() {
    Item::counter = 0;
    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        ptr.Reset(new Item);
        ASSERT_EQUAL(Item::counter, 1);
    }
    ASSERT_EQUAL(Item::counter, 0);

    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        auto rawPtr = ptr.Release();
        ASSERT_EQUAL(Item::counter, 1);

        delete rawPtr;
        ASSERT_EQUAL(Item::counter, 0);
    }
    ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
    UniquePtr<Item> ptr(new Item(42));
    ASSERT_EQUAL(ptr.Get()->value, 42);
    ASSERT_EQUAL((*ptr).value, 42);
    ASSERT_EQUAL(ptr->value, 42);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestLifetime);
    RUN_TEST(tr, TestGetters);
}