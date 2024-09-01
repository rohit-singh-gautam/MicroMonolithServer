/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/error.h>

namespace MMS {

class ConstStream;
class Stream {
protected:
    friend class ConstStream;
    mutable uint8_t *_curr;
    uint8_t * _end;

    void CheckOverflow() const { if (_curr >= _end) throw StreamOverflowException { }; }
    void CheckOverflow(size_t len) const { if (_curr + len > _end) throw StreamOverflowException { }; }

    Stream() : _curr { nullptr }, _end { nullptr } { }
public:
    constexpr Stream(auto *_begin, auto *_end) : _curr { reinterpret_cast<uint8_t *>(_begin) }, _end { reinterpret_cast<uint8_t *>(_end) } { }
    constexpr Stream(auto *_begin, size_t size) : _curr { reinterpret_cast<uint8_t *>(_begin) }, _end { _curr + size } { }
    constexpr Stream(Stream &&stream) : _curr { stream._curr }, _end { stream._end } { stream._curr = stream._end = nullptr; }
    Stream(const Stream &stream) : _curr { stream._curr }, _end { stream._end } { }
    virtual ~Stream() = default;

    constexpr Stream GetSimpleStream() { return Stream { _curr, _end }; }
    constexpr const Stream GetSimpleStream() const { return Stream { _curr, _end }; }
    constexpr ConstStream GetSimpleConstStream();
    constexpr const ConstStream GetSimpleConstStream() const;

    Stream &operator=(const Stream &stream) { _curr = stream._curr; return *this; }
    const Stream &operator=(const Stream &stream) const { _curr = stream._curr; return *this; }
    virtual Stream operator+(size_t len) { _curr += len; return *this; }
    virtual const Stream operator+(size_t len) const { _curr += len; return *this; }
    virtual Stream &operator+=(size_t len) { _curr += len; return *this; }
    virtual const Stream &operator+=(size_t len) const { _curr += len; return *this; }

    virtual constexpr inline uint8_t &operator*() { return *_curr; }
    virtual constexpr inline uint8_t operator*() const { return *_curr; }
    virtual constexpr inline Stream &operator++() { ++_curr; return *this; }
    virtual constexpr inline Stream &operator--() { --_curr; return *this;}
    virtual constexpr inline const Stream &operator++() const { ++_curr; return *this; }
    virtual constexpr inline const Stream &operator--() const { --_curr; return *this;}

    virtual constexpr inline uint8_t *GetCurrAndIncrease(const size_t len) { auto temp = _curr; _curr += len; return temp; }
    virtual constexpr inline const uint8_t *GetCurrAndIncrease(const size_t len) const { auto temp = _curr; _curr += len; return temp; }

    size_t GetSizeFrom(const auto *start) const { return static_cast<size_t>(_curr - reinterpret_cast<const uint8_t *>(start)); }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
    virtual constexpr inline uint8_t *operator++(int) { uint8_t *temp = _curr; ++_curr; return temp; };
    virtual constexpr inline uint8_t *operator--(int) { uint8_t *temp = _curr; --_curr; return temp; };
    virtual constexpr inline const uint8_t *operator++(int) const { uint8_t *temp = _curr; ++_curr; return temp; };
    virtual constexpr inline const uint8_t *operator--(int) const { uint8_t *temp = _curr; --_curr; return temp; };
    #pragma GCC diagnostic pop

    const auto end() const { return _end; }
    const auto curr() const { return _curr; }

    auto &end() { return _end; }
    auto &curr() { return _curr; }

    auto remaining_buffer() const { return static_cast<size_t>(_end - _curr); }

    bool full() const { return _curr == _end; }

    template <typename T>
    bool CheckCapacity() const { return remaining_buffer() >= sizeof(T); }
    bool CheckCapacity(const size_t size) const { return remaining_buffer() >= size; }

    void UpdateCurr(uint8_t *_curr) const { this->_curr = _curr; }

    auto GetRawCurrentBuffer() { return std::make_pair(_curr, remaining_buffer()); }

    virtual constexpr inline void Reserve(const size_t len) { CheckOverflow(len); }
    constexpr inline void Copy(const ConstStream &source);
    constexpr inline void Copy(const auto *begin, const auto *end) { Reserve(static_cast<size_t>(end - begin)); _curr = std::copy(reinterpret_cast<const uint8_t *>(begin), reinterpret_cast<const uint8_t *>(end), _curr); }
    constexpr inline void Copy(const auto *begin, size_t size) { Reserve(size); _curr = std::copy(reinterpret_cast<const uint8_t *>(begin), reinterpret_cast<const uint8_t *>(begin) + size, _curr); }
};

class ConstStream {
protected:
    mutable const uint8_t *_curr;
    const uint8_t * _end;

    void CheckOverflow() const { if (_curr >= _end) throw StreamOverflowException { }; }

public:
    constexpr ConstStream(const auto *_begin, const auto *_end) : _curr { reinterpret_cast<const uint8_t *>(_begin) }, _end { reinterpret_cast<const uint8_t *>(_end) } { }
    constexpr ConstStream(const auto *_begin, size_t size) : _curr { reinterpret_cast<const uint8_t *>(_begin) }, _end { reinterpret_cast<const uint8_t *>(_begin) + size } { }
    constexpr ConstStream(Stream &&stream) : _curr { stream._curr }, _end { stream._end } { stream._curr = stream._end = nullptr; }
    constexpr ConstStream(const std::string &string) : _curr { reinterpret_cast<const uint8_t *>(string.c_str()) }, _end { _curr + string.size() } { }
    ConstStream(const Stream &stream) : _curr { stream._curr }, _end { stream._end } { }
    ConstStream(const ConstStream &stream) : _curr { stream._curr }, _end { stream._end } { }
    virtual ~ConstStream() = default;

    constexpr ConstStream GetSimpleStream() { return ConstStream { _curr, _end }; }
    constexpr const ConstStream GetSimpleStream() const { return ConstStream { _curr, _end }; }
    constexpr ConstStream GetSimpleConstStream() { return ConstStream { _curr, _end }; }
    constexpr const ConstStream GetSimpleConstStream() const { return ConstStream { _curr, _end }; }

    template <size_t _size>
    bool operator==(const auto (&data)[_size + 1]) const {
        if (remaining_buffer() < _size) return false;
        return std::equal(std::begin(data), std::end(data), _curr);
    }

    /*! IMPORTANT: text must not be null terminated */
    bool operator==(const std::string_view &text) const {
        if (remaining_buffer() < text.size()) return false;
        return std::equal(std::begin(text), std::end(text), _curr);
    }

    ConstStream &operator=(const ConstStream &stream) { _curr = stream._curr; return *this; }
    const ConstStream &operator=(const ConstStream &stream) const { _curr = stream._curr; return *this; }
    virtual ConstStream operator+(size_t len) { _curr += len; return *this; }
    virtual const ConstStream operator+(size_t len) const { _curr += len; return *this; }
    virtual ConstStream &operator+=(size_t len) { _curr += len; return *this; }
    virtual const ConstStream &operator+=(size_t len) const { _curr += len; return *this; }

    virtual constexpr inline uint8_t operator*() const { return *_curr; }
    virtual constexpr inline ConstStream &operator++() { ++_curr; return *this; }
    virtual constexpr inline ConstStream &operator--() { --_curr; return *this;}
    virtual constexpr inline const ConstStream &operator++() const { ++_curr; return *this; }
    virtual constexpr inline const ConstStream &operator--() const { --_curr; return *this;}

    virtual constexpr inline const uint8_t *GetCurrAndIncrease(const size_t len) { auto temp = _curr; _curr += len; return temp; }
    virtual constexpr inline const uint8_t *GetCurrAndIncrease(const size_t len) const { auto temp = _curr; _curr += len; return temp; }

    size_t GetSizeFrom(const auto *start) const { return static_cast<size_t>(_curr - reinterpret_cast<const uint8_t *>(start)); }


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
    virtual constexpr inline const uint8_t *operator++(int) const { const uint8_t *temp = _curr; ++_curr; return temp; };
    virtual constexpr inline const uint8_t *operator--(int) const { const uint8_t *temp = _curr; --_curr; return temp; };
    #pragma GCC diagnostic pop

    const auto end() const { return _end; }
    const auto curr() const { return _curr; }

    auto &end() { return _end; }
    auto &curr() { return _curr; }

    size_t remaining_buffer() const { return static_cast<size_t>(_end - _curr); }

    bool full() const { return _curr == _end; }

    template <typename T>
    bool CheckCapacity() const { return remaining_buffer() >= sizeof(T); }
    bool CheckCapacity(const size_t size) const { return remaining_buffer() >= size; }

    void UpdateCurr(uint8_t *_curr) const { this->_curr = _curr; }
    auto GetRawCurrentBuffer() const { return std::make_pair(_curr, remaining_buffer()); }
};

constexpr inline void Stream::Copy(const ConstStream &source) {
    Reserve(source.remaining_buffer());
    _curr = std::copy(source.curr(), source.end(), _curr);
}

constexpr ConstStream Stream::GetSimpleConstStream() { return ConstStream { _curr, _end }; }
constexpr const ConstStream Stream::GetSimpleConstStream() const { return ConstStream { _curr, _end }; }


class StreamChecked : public Stream {
public:
    using Stream::Stream;
    constexpr inline uint8_t &operator*() override { CheckOverflow(); return *_curr; }
    constexpr inline uint8_t operator*() const override { CheckOverflow(); return *_curr; }
    constexpr inline void Reserve(const size_t) override { }
};

class FullStream : public Stream {
protected:
    uint8_t * _begin;

    void CheckUnderflow() const { if (_curr == _begin) throw StreamUnderflowException { }; }

    FullStream() : Stream { }, _begin { nullptr } { }
public:
    constexpr FullStream(auto *_begin, auto *_end) : Stream {reinterpret_cast<uint8_t *>(_begin), reinterpret_cast<uint8_t *>(_end)}, _begin { reinterpret_cast<uint8_t *>(_begin) } { }
    constexpr FullStream(auto *_begin, auto *_end, auto *_curr) : Stream {reinterpret_cast<uint8_t *>(_curr), reinterpret_cast<uint8_t *>(_end)}, _begin { reinterpret_cast<uint8_t *>(_begin) } { }
    constexpr FullStream(auto *_begin, size_t size) :  Stream {reinterpret_cast<uint8_t *>(_begin), size}, _begin { reinterpret_cast<uint8_t *>(_begin) } { }
    constexpr FullStream(FullStream &&stream) : Stream { std::move(stream) }, _begin { stream._begin } { stream._begin = nullptr; }
    FullStream(const FullStream &stream) : Stream { stream }, _begin { stream._begin } { }

    FullStream &operator=(const FullStream &stream) { _curr = stream._curr; return *this; }

    const auto begin() const { return _begin; }
    auto begin() { return _begin; }

    auto index() const { return static_cast<size_t>(_curr - _begin); }
    auto capacity() const { return static_cast<size_t>(_end - _begin); }

    uint8_t *Move() {
        auto temp = _begin;
        _begin = _end = _curr = nullptr;
        return temp;
    }

    bool IsNull() { return _begin == nullptr; }
    auto GetRawFullBuffer() { return std::make_pair(_begin, capacity()); }
    void Reset() { _curr = _begin; }
    bool empty() { return _curr == _begin; }
};

class ConstFullStream : public ConstStream {
protected:
    const uint8_t * _begin;

    void CheckUnderflow() const { if (_curr == _begin) throw StreamUnderflowException { }; }
public:
    constexpr ConstFullStream(const auto *_begin, const auto *_end) : ConstStream {reinterpret_cast<const uint8_t *>(_begin), reinterpret_cast<const uint8_t *>(_end)}, _begin { reinterpret_cast<const uint8_t *>(_begin) } { }
    constexpr ConstFullStream(const auto *_begin, size_t size) :  ConstStream {reinterpret_cast<const uint8_t *>(_begin), size}, _begin { reinterpret_cast<const uint8_t *>(_begin) } { }
    constexpr ConstFullStream(const std::string &string) :  ConstStream {reinterpret_cast<const uint8_t *>(string.c_str()), string.size()}, _begin { _curr } { }
    constexpr ConstFullStream(ConstFullStream &&stream) : ConstStream { std::move(stream) }, _begin { stream._begin } { stream._begin = nullptr; }
    ConstFullStream(const ConstFullStream &stream) : ConstStream { stream }, _begin { stream._begin } { }

    ConstFullStream &operator=(const ConstFullStream &stream) { _curr = stream._curr; return *this; }

    const auto begin() const { return _begin; }
    auto begin() { return _begin; }

    auto index() const { return static_cast<size_t>(_curr - _begin); }
    auto capacity() const { return static_cast<size_t>(_end - _begin); }

    const uint8_t *Move() {
        auto temp = _begin;
        _begin = _end = _curr = nullptr;
        return temp;
    }

    bool IsNull() { return _begin == nullptr; }
    auto GetRawFullBuffer() { return std::make_pair(_begin, capacity()); }
    void Reset() { _curr = _begin; }
    bool empty() { return _curr == _begin; }
};

class FullStreamChecked : public FullStream {
public:
    using FullStream::FullStream;
    constexpr inline uint8_t &operator*() override { CheckOverflow(); return *_curr; }
    constexpr inline uint8_t operator*() const override { CheckOverflow(); return *_curr; }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
    constexpr inline Stream &operator--() override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline const Stream &operator--() const override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline uint8_t *operator--(int) override { CheckUnderflow(); uint8_t *temp = _curr; --_curr; return temp; };
    constexpr inline const uint8_t *operator--(int) const override { CheckUnderflow(); uint8_t *temp = _curr; --_curr; return temp; };
    #pragma GCC diagnostic pop
};

class ConstFullStreamChecked : public ConstFullStream {
public:
    using ConstFullStream::ConstFullStream;
    constexpr inline uint8_t operator*() const override { CheckOverflow(); return *_curr; }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
    constexpr inline ConstStream &operator--() override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline const ConstStream &operator--() const override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline const uint8_t *operator--(int) const override { CheckUnderflow(); const uint8_t *temp = _curr; --_curr; return temp; };
    #pragma GCC diagnostic pop
};

class FullStreamAutoAlloc : public FullStream {
    constexpr inline void CheckResize() {
        if (_curr == _end) {
            auto curr_index = index();
            auto new_capacity = capacity() * 2;
            _begin = reinterpret_cast<uint8_t *>(realloc(reinterpret_cast<void *>(_begin), new_capacity));
            _end = _begin + new_capacity;
            _curr = _begin + curr_index;
        }
    }

    constexpr inline void CheckResize(const size_t len) {
        if (_curr + len > _end) {
            auto curr_index = index();
            auto new_capacity = capacity() * 2;
            while(curr_index + len > new_capacity) new_capacity += capacity();
            _begin = reinterpret_cast<uint8_t *>(realloc(reinterpret_cast<void *>(_begin), new_capacity));
            if (_begin == nullptr) throw MemoryAllocationException { };
            _end = _begin + new_capacity;
            _curr = _begin + curr_index;
        }
    }

public:
    using FullStream::FullStream;
    FullStreamAutoAlloc(const size_t size) : FullStream { reinterpret_cast<uint8_t *>(malloc(size)), size } { if (_begin == nullptr) throw MemoryAllocationException { }; }
    FullStreamAutoAlloc() : FullStream { } { }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
    constexpr inline Stream &operator--() override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline const Stream &operator--() const override { CheckUnderflow(); --_curr; return *this;}
    constexpr inline Stream &operator++() override { CheckResize(); ++_curr; return *this; }
    constexpr inline const Stream &operator++() const override { CheckOverflow(); ++_curr; return *this; }
    constexpr inline uint8_t *operator++(int) override { CheckResize(); uint8_t *temp = _curr; ++_curr; return temp; };
    constexpr inline const uint8_t *operator++(int) const override { CheckOverflow(); uint8_t *temp = _curr; ++_curr; return temp; };
    constexpr inline Stream operator+(size_t len) override { CheckResize(len); _curr += len; return *this; }
    constexpr inline  const Stream operator+(size_t len) const override { _curr += len; CheckOverflow(); return *this; }
    constexpr inline Stream &operator+=(size_t len) override { CheckResize(len); _curr += len; return *this; }
    constexpr inline const Stream &operator+=(size_t len) const override { _curr += len; CheckOverflow(); return *this; }
    #pragma GCC diagnostic pop


    constexpr inline void Reserve(const size_t len) override { CheckResize(len); }

    constexpr inline uint8_t *GetCurrAndIncrease(const size_t len) override { CheckResize(len); auto temp = _curr; _curr += len; return temp; }
    constexpr inline const uint8_t *GetCurrAndIncrease(const size_t len) const override { auto temp = _curr; _curr += len; CheckOverflow(); return temp; }

    auto ReturnOldAndAlloc(const size_t size) { 
        FullStream stream { _begin, _end, _curr };
        _begin = reinterpret_cast<uint8_t *>(malloc(size));
        if (_begin == nullptr) throw MemoryAllocationException { }; 
        _curr = _begin;
        _end = _begin + size;
        return stream;
    }
};


} // namespace MMS