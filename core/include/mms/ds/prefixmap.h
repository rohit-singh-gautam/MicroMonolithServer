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
        prefixentry() { }
        prefixentry(prefixentry &&pe) : value { std::move(pe.value) }, match { std::move(pe.match) }, children { std::move(children) } { }
        prefixentry(const prefixentry &) = delete;
        prefixentry &operator=(const prefixentry &) = delete;
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
    prefixmap() { }
    prefixmap(prefixmap &&pm) : root { std::move(pm.root) } { }
    prefixmap(const prefixmap &) = delete;
    prefixmap &operator=(const prefixmap &) = delete;

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
    auto &search(const stringtype &key, stringtype &newkey) const {
        size_t index { 0 };
        auto current = &root;
        decltype(current) prev = nullptr;
        size_t previndex { 0 };
        for(;;) {
            auto &match = current->match;
            size_t matchindex { 0 };
            while(matchindex < match.size() && index < key.size() && match[matchindex] == key[index]) {
                ++matchindex;
                ++index;
            }

            if (matchindex < match.size()) {
                if (prev) {
                    newkey = key.substr(previndex, key.size() - previndex);
                    return prev->value;
                }
                break;
            }
            auto childitr = current->children.find(key[index]);
            if (index == key.size()) return current->value;
            if (childitr == std::end(current->children)) {
                newkey = key.substr(index, key.size() - index);
                return current->value;
            }
            prev = current;
            previndex = index;
            current = childitr->second.get();
            ++index;
        }
        return emptyvalue;
    }
};

} // namespace MMS