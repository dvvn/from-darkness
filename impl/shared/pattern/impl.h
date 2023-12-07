﻿#pragma once
#include "container/array.h"
#include "container/span.h"
#include "diagnostics/fatal.h"
#include "functional/bind.h"
#include "pattern/fwd.h"
#include "concepts.h"
#include "noncopyable.h"

#include <boost/hana/front.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/unpack.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace fd
{
template <class Segment>
struct pattern_segment_constant_size;

template <>
struct pattern_segment_constant_size<pattern_segment<-1, -1>>;

template <pattern_size_type UnknownBytes>
struct pattern_segment_constant_size<pattern_segment<-1, UnknownBytes>>;

template <pattern_size_type Bytes>
struct pattern_segment_constant_size<pattern_segment<Bytes, -1>>;

template <pattern_size_type Bytes, pattern_size_type UnknownBytes>
struct pattern_segment_constant_size<pattern_segment<Bytes, UnknownBytes>>
{
    static constexpr pattern_size_type known   = Bytes;
    static constexpr pattern_size_type unknown = UnknownBytes;

    static constexpr pattern_size_type size = Bytes + UnknownBytes;
};

template <pattern_size_type BytesCount>
struct pattern_segment_bytes : array<uint8_t, BytesCount>, noncopyable
{
    using size_type = pattern_size_type;

    template <typename It>
    constexpr pattern_segment_bytes(It from)
    {
        if constexpr (BytesCount == 1)
            this->front() = *from;
        else if constexpr (std::random_access_iterator<It>)
            std::copy(from, from + BytesCount, this->data());
        else
            std::copy_n(from, BytesCount, this->data());
    }

    pattern_segment_bytes(pattern_segment_bytes&& other) noexcept
        : array<uint8_t, BytesCount>{std::move(other)}
    {
    }

    pattern_segment_bytes& operator=(pattern_segment_bytes&& other) noexcept
    {
        this->swap(other);
        return *this;
    }

    static constexpr size_type size()
    {
        return BytesCount;
    }

    static constexpr size_type max_size()
    {
        return BytesCount;
    }
};

template <>
struct pattern_segment_bytes<0>;

template <>
struct pattern_segment_bytes<-1> : span<char const>
{
    using size_type = pattern_size_type;
    using span::span;
};

template <pattern_size_type BytesCount>
struct pattern_segment_unknown_bytes : std::integral_constant<pattern_size_type, BytesCount>
{
    constexpr pattern_segment_unknown_bytes(std::integral_constant<pattern_size_type, BytesCount> = {})
    {
    }
};

template <>
struct pattern_segment_unknown_bytes<-1>
{
    using value_type = pattern_size_type;

    value_type value;

    template <value_type BytesCount>
    constexpr pattern_segment_unknown_bytes(std::integral_constant<value_type, BytesCount>)
        : value{BytesCount}
    {
    }

    constexpr pattern_segment_unknown_bytes(value_type const bytes_count)
        : value{bytes_count}
    {
    }
};

template <pattern_size_type Bytes, pattern_size_type UnknownBytes>
struct pattern_segment
{
    template <class... Segment>
    friend struct pattern;

    using known_bytes_storage   = pattern_segment_bytes<Bytes>;
    using unknown_bytes_storage = pattern_segment_unknown_bytes<UnknownBytes>;

    using size_type      = pattern_size_type;
    using known_iterator = typename known_bytes_storage::const_iterator;
    using known_pointer  = typename known_bytes_storage::const_pointer;

  private:
    known_bytes_storage known_bytes_;
    [[no_unique_address]] //
    unknown_bytes_storage unknown_bytes_;

  public:
    constexpr pattern_segment(known_bytes_storage known_bytes, unknown_bytes_storage unknown_bytes = {})
        : known_bytes_{std::move(known_bytes)}
        , unknown_bytes_{std::move(unknown_bytes)}
    {
    }

#if 0
    constexpr known_iterator begin() const
    {
        return known_bytes_.begin();
    }

    constexpr known_iterator end() const
    {
        return known_bytes_.end();
    }

    constexpr size_type size() const
    {
        return known_bytes_.size();
    }

#ifdef _MSC_VER
    constexpr known_pointer _Unchecked_begin() const noexcept
    {
        return known_bytes_._Unchecked_begin();
    }

    constexpr known_pointer _Unchecked_end() const noexcept
    {
        return known_bytes_._Unchecked_end();
    }
#endif
#endif

    [[deprecated]]
    constexpr known_bytes_storage const& view() const
    {
        return known_bytes_;
    }

    [[deprecated]]
    constexpr size_type unknown() const
    {
        return unknown_bytes_.value;
    }

    template <typename It>
    bool equal(It it) const
    {
        auto const bytes_first  = known_bytes_.data();
        auto const bytes_length = known_bytes_.size();
        return std::equal(bytes_first, bytes_first + bytes_length, it);
    }

    constexpr size_type length() const
    {
        return known_bytes_.size() + unknown_bytes_.value;
    }
};

template <class... Segment>
struct pattern
{
    using size_type = pattern_size_type;

  private:
    using storage_type = boost::hana::tuple<Segment...>;

    storage_type bytes_;

  public:
    constexpr pattern(Segment... segment)
        : bytes_(std::move(segment)...)
    {
    }

    [[deprecated]]
    constexpr auto get() const -> storage_type const&
    {
        return bytes_;
    }

    constexpr size_type length() const
    {
#ifdef _DEBUG
        if constexpr ((complete<pattern_segment_constant_size<Segment>> && ...))
            return (pattern_segment_constant_size<Segment>::size + ...);
        else
#endif
            return boost::hana::unpack(bytes_, [](Segment const&... segment) -> size_type {
                return (segment.length() + ...);
            });
    }

    constexpr auto& front() const
    {
        return boost::hana::front(bytes_).known_bytes_;
    }

    template <typename T>
    bool equal(T const* mem) const
    {
        auto const equal_impl = [&mem](auto& self, auto& segment, auto&... next) -> bool {
            if (!segment.equal(mem))
                return false;
            if constexpr (sizeof...(next) == 0)
                return true;
            else
            {
                mem += segment.length();
                return self(self, next...);
            }
        };
        auto const equal_impl_ref = std::ref(equal_impl);
        return boost::hana::unpack(bytes_, bind_front(equal_impl_ref, equal_impl_ref));
    }
};
} // namespace fd
