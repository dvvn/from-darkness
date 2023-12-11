#pragma once
#include "container/array.h"
#include "container/span.h"
#include "pattern/basic_holder.h"
#include "concepts.h"
#include "noncopyable.h"

#include <boost/hana/tuple.hpp>
#include <boost/hana/unpack.hpp>

#include <algorithm>

namespace fd
{
template <size_t BytesCount>
struct pattern_segment_bytes : array<uint8_t, BytesCount>, noncopyable
{
#ifdef FD_PATTERN_NATIVE_SIZE
    using size_type = size_t;
#else
    using size_type = small_type<size_t, BytesCount>;
#endif

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

    constexpr pattern_segment_bytes(pattern_segment_bytes&& other) noexcept
        : array<uint8_t, BytesCount>{std::move(other)}
    {
    }

    constexpr pattern_segment_bytes& operator=(pattern_segment_bytes&& other) noexcept
    {
        this->swap(other);
        return *this;
    }

    static constexpr integral_constant<size_type, BytesCount> size()
    {
        return {};
    }

    static constexpr integral_constant<size_type, BytesCount> max_size()
    {
        return {};
    }
};

template <>
struct pattern_segment_bytes<0>;

template <>
struct pattern_segment_bytes<-1> : span<char const>
{
    using span::span;
};

#ifdef FD_PATTERN_NATIVE_SIZE
template <size_t BytesCount>
struct pattern_segment_unknown_bytes : integral_constant<size_t, BytesCount>
{
    using size_type = size_t;

    constexpr pattern_segment_unknown_bytes(integral_constant<size_t, BytesCount> = {})
    {
    }
};
#else
template <size_t BytesCount>
struct pattern_segment_unknown_bytes : integral_constant<small_type<size_t, BytesCount>, BytesCount>
{
    using size_type = small_type<size_t, BytesCount>;

    template <typename N>
    constexpr pattern_segment_unknown_bytes(integral_constant<N, BytesCount>)
    {
    }

    pattern_segment_unknown_bytes() = default;
};
#endif
template <>
struct pattern_segment_unknown_bytes<-1>
{
    using value_type = size_t;
    using size_type  = size_t;

    value_type value;

    constexpr operator value_type() const noexcept
    {
        return value;
    }

    template <value_type BytesCount>
    constexpr pattern_segment_unknown_bytes(integral_constant<value_type, BytesCount>)
        : value{BytesCount}
    {
    }

    constexpr pattern_segment_unknown_bytes(value_type const bytes_count)
        : value{bytes_count}
    {
    }
};

template <size_t Bytes, size_t UnknownBytes>
struct pattern_segment
{
    using known_bytes_storage   = pattern_segment_bytes<Bytes>;
    using unknown_bytes_storage = pattern_segment_unknown_bytes<UnknownBytes>;

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

    constexpr known_bytes_storage const& get() const
    {
        return known_bytes_;
    }

    constexpr auto known() const -> typename known_bytes_storage::size_type
    {
        return known_bytes_.size();
    }

    constexpr auto unknown() const -> typename unknown_bytes_storage::size_type
    {
        return unknown_bytes_;
    }
};

template <class... Segment>
class pattern
{
    using storage_type = boost::hana::tuple<Segment...>;

    storage_type bytes_;

  public:
    constexpr pattern(Segment... segment)
        : bytes_(std::move(segment)...)
    {
    }

    constexpr storage_type const& get() const
    {
        return bytes_;
    }

    template <size_t Index>
    constexpr auto& get() const
    {
        return boost::hana::at_c<Index>(bytes_);
    }

    constexpr auto length() const
    {
        if constexpr ((complete<pattern_segment_constant_size<Segment>> && ...))
            return (pattern_segment_constant_size<Segment>::length + ...);
        else if constexpr (sizeof...(Segment) == 1)
            return get<0>().length();
        else
            return boost::hana::unpack(bytes_, [](Segment const&... segment) -> size_t {
                return (static_cast<size_t>(segment.length()) + ...);
            });
    }
};
} // namespace fd
