/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <unordered_map>
#include <memory>
#include <string>

namespace MMS {

template <typename stringtype, typename StoreType>
class prefixmap {
    // Private structure doest not require to be used beyond this class
    struct prefixentry {
        StoreType value { };
        stringtype match { };
        std::unordered_map<char, std::unique_ptr<prefixentry>> children { };
    };

    StoreType emptyvalue { };
    prefixentry root { };

    auto breakmatch(prefixentry *current, size_t breakindex) {
        auto &match = current->match;
        char ch = match[breakindex];
        auto prematch = match.substr(0, breakindex);
        auto postmatch = match.substr(breakindex + 1, match.size() - breakindex - 1);

        auto newentry = std::make_unique<prefixentry>();
        current->match = std::move(prematch);
        newentry->match = std::move(postmatch);
        std::swap(newentry->value, current->value);
        std::swap(newentry->children, current->children);
        current->children[ch] = std::move(newentry);
        return newentry.get();
    }

public:

    void insert(const stringtype &key, const StoreType &value) {
        auto copyvalue = value;
        insert(key, std::move(copyvalue));
    }


    void insert(const stringtype &key, StoreType &&value) {
        size_t index { 0 };

        prefixentry *current = &root;

        if (!root.match.empty() || !root.children.empty()) {
            for(;;) {
                auto &match = current->match;
                size_t matchindex { 0 };
                while(matchindex <= match.size() && index < key.size() && match[matchindex] == key[index]) {
                    ++matchindex;
                    ++index;
                }

                if (matchindex < match.size()) {
                    breakmatch(current, matchindex);
                }

                if (index == key.size()) break;
                if (current->children.contains(key[index])) {
                    current = current->children[key[index]].get();
                    ++index;
                } else {
                    auto newentry = std::make_unique<prefixentry>();
                    newentry->match = key.substr(index + 1, key.size() - index - 1);
                    auto newentryptr = newentry.get();
                    current->children[key[index]] = std::move(newentry);
                    current = newentryptr;
                    break;
                }

            }
        } else {
            current->match = key;
        }

        current->value = std::move(value);
    }

    /*! Search will return value for largest prefix match */
    auto &search(const stringtype &key) {
        size_t index { 0 };
        prefixentry *current = &root;
        prefixentry *prev = nullptr;
        for(;;) {
            auto &match = current->match;
            size_t matchindex { 0 };
            while(matchindex < match.size() && index < key.size() && match[matchindex] == key[index]) {
                ++matchindex;
                ++index;
            }

            if (matchindex < match.size()) {
                if (prev) return prev->value;
                break;
            }
            if (index == key.size() || !current->children.contains(key[index])) return current->value;
            prev = current;
            current = current->children[key[index]].get();
            ++index;
        }
        return emptyvalue;
    }
};

} // namespace MMS