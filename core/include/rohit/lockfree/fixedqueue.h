/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>

namespace MMS {

namespace lockfree {

/*! Lock
 *
 */
template <typename EntryType>
class fixedqueue_t {
    const size_t max_size;
    std::unique_ptr<EntryType[]> storage;

    // StartIndex == EndIndex means empty
    volatile size_t StartIndex { 0 };
    volatile size_t EndIndex { 0 };

public:
    fixedqueue_t(const size_t size) : max_size { size }, storage { std::make_unique<EntryType[]>(size) } { }

    /*! This function must be used by publisher thread */
    constexpr inline bool full() const {
        return ((EndIndex + 1) % max_size) == StartIndex;
    }

    /*! This function must be used by subscriber thread */
    constexpr inline bool empty() const {
        return StartIndex == EndIndex;
    }

    /*! This is approximate function does not gaurantee exact result */
    constexpr inline auto size() const {
        return (max_size + EndIndex - StartIndex) % max_size;
    }

    constexpr inline auto maxsize() const { return max_size; }

    /*! This function must be used by publisher thread only. */
    constexpr bool push_back(const EntryType &value) {
        const auto NextEndIndex = (EndIndex + 1) % max_size;
        if (NextEndIndex != StartIndex) {
            /* First add then increase pointer */
            storage[EndIndex] = value;
            EndIndex = NextEndIndex;
            return true;
        } else return false;
    }

    /*! This function must be used by publisher thread only. */
    constexpr void push_back_unsafe(const EntryType &value) {
        const auto NextEndIndex = (EndIndex + 1) % max_size;
        /* First add then increase pointer */
        storage[EndIndex] = value;
        EndIndex = NextEndIndex;
    }

    /*! This function must be used by publisher thread only. */
    constexpr bool push_back(EntryType &&value) {
        const auto NextEndIndex = (EndIndex + 1) % max_size;
        if (NextEndIndex != StartIndex) {
            /* First add then increase pointer */
            storage[EndIndex] = std::move(value);
            EndIndex = NextEndIndex;
            return true;
        } else return false;
    }

    /*! This function must be used by subscriber thread only. */
    constexpr bool pop_front() {
        if (!empty) {
            StartIndex = (StartIndex + 1) % max_size;
            return true;
        } else return false;
    }

    /*! This function must be used by subscriber thread only. Also, this is unsafe function must be checked if elements are present before using it*/
    constexpr EntryType &front() {
        return storage[StartIndex];
    }

    constexpr const EntryType &front() const {
        return storage[StartIndex];
    }

    /*! This function must be used by subscriber thread only. Also, this is unsafe function must be checked if elements are present before using it*/
    constexpr EntryType get_pop_front() {
        auto ret = storage[StartIndex];
        StartIndex = (StartIndex + 1) % max_size;
        return ret;
    }

    /*! This function must be used by subscriber thread only.
     *  Also, this is unsafe function must be checked if elements are present before using it.
     */
    constexpr EntryType &operator[](const auto &RelativeIndex) {
        const auto AbsoluteIndex = (StartIndex + RelativeIndex) % max_size;
        return storage[RelativeIndex];
    }

    constexpr const EntryType &operator[](const auto &RelativeIndex) const {
        const auto AbsoluteIndex = (StartIndex + RelativeIndex) % max_size;
        return storage[RelativeIndex];
    }

    /*! This function must be used by subscriber thread only. This is alernate to !IsEmpty(). */
    operator bool() const { return StartIndex != EndIndex; }
};

} // namespace lockfree

} // namespace MMS