//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <memory>
#include <vector>
#include <functional>
#include <utility>

namespace MMS::server {
template <typename Key, typename Value>
class SplitOrderMap {
protected:
    struct Node {
        Key key;
        Value value;
        Node* next;

        Node(const Key& k, const Value& v) : key { k }, value { v }, next { nullptr } {}
        Node(const Key& k, Value&& v) : key { k }, value { std::move(v) }, next { nullptr } {}
        Node(Key&& k, const Value& v) : key { std::move(k) }, value { v }, next { nullptr } {}
        Node(Key&& k, Value&& v) : key { std::move(k) }, value { std::move(v) }, next { nullptr } {}
        Node(const Node &) = delete;
        Node& operator=(const Node &) = delete;
    };

    struct Bucket {
        std::mutex lock { };
        Node* head { nullptr };

        Bucket() = default;
        Bucket(const Bucket &rhs) : head { rhs.head } { }
        Bucket& operator=(const Bucket &rhs) { head = rhs.head; return *this; }
    };

    std::vector<Bucket> table;
    std::atomic<size_t> size;
    size_t capacity;

public:
    SplitOrderMap(size_t initial_capacity = 16)
        : table { }, size { 0 }, capacity { initial_capacity } {
            table.resize(capacity, { });
    }

    SplitOrderMap(const SplitOrderMap& rhs) = default;
    SplitOrderMap(SplitOrderMap&& rhs) : table { std::move(rhs.table) }, size { rhs.size.load() }, capacity { rhs.capacity }  { }
    SplitOrderMap& operator=(const SplitOrderMap& rhs) = default;

    constexpr size_t get_index(const Key& key) const {
        size_t hash = std::hash<Key>{}(key);
        return hash % capacity;
    }

    bool insert(size_t index, Node *new_node) {
        auto &bucket = table[index];
        std::lock_guard<std::mutex> lock(bucket.lock);
        while (true) {
            if (bucket.head == nullptr) {
                bucket.head = new_node;
                size.fetch_add(1, std::memory_order_relaxed);
                return true;
            } else {
                Node* current = bucket.head;
                while (current) {
                    if (current->key == new_node->key) {
                        delete new_node;
                        return false;
                    }
                    current = current->next;
                }
                new_node->next = bucket.head;
                bucket.head = new_node;
                size.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
    }

    bool insert(const Key& key, const Value& value) {
        size_t index = get_index(key);
        auto new_node = new Node(key, value);
        return insert(index, new_node);
    }


    bool insert(const Key& key, Value&& value) {
        size_t index = get_index(key);
        auto new_node = new Node(key, std::move(value));
        return insert(index, new_node);
    }

    bool insert(Key&& key, Value&& value) {
        size_t index = get_index(key);
        auto new_node = new Node(std::move(key), std::move(value));
        return insert(index, new_node);
    }

    bool remove(const Key& key) {
        size_t index = get_index(key);
        auto &bucket = table[index];
        std::lock_guard<std::mutex> lock(bucket.lock);

        Node* current = bucket.head;
        Node* prev = nullptr;

        while (current) {
            if (current->key == key) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    bucket.head = current->next;
                }
                delete current;
                size.fetch_sub(1, std::memory_order_relaxed);
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    std::optional<std::reference_wrapper<Value>> find(const Key& key) {
        size_t index = get_index(key);
        auto &bucket = table[index];
        std::lock_guard<std::mutex> lock(bucket.lock);

        Node* current = bucket.head;
        while (current) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        return std::nullopt;
    }

    bool contains(const Key& key) {
        size_t index = get_index(key);
        auto &bucket = table[index];
        std::lock_guard<std::mutex> lock(bucket.lock);

        Node* current = bucket.head;
        while (current) {
            if (current->key == key) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    size_t get_size() const {
        return size.load(std::memory_order_relaxed);
    }

}; // class SplitOrderMap

} // namespace MMS::server