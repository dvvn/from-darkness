#pragma once

#include "container/array.h"
#include "container/span.h"
#include "diagnostics/fatal.h"
#include "iterator/unwrap.h"
#include "memory/pattern_fwd.h"
#include "string/charconv.h"

#include <boost/hana/append.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace fd
{
namespace detail
{
template <pattern_size_type S>
struct pattern_string
{
    char buff[S];

    template <pattern_size_type... I>
    consteval pattern_string(char const* buff, std::index_sequence<I...>)
        : buff{buff[I]...}
    {
    }

    consteval pattern_string(char const* buff)
        : pattern_string(buff, std::make_index_sequence<S>())
    {
    }

    //--

    // ReSharper disable once CppMemberFunctionMayBeStatic
    constexpr pattern_size_type length() const
    {
        return S;
    }

    constexpr char const* begin() const
    {
        return std::begin(buff);
    }

    constexpr char const* end() const
    {
        return std::end(buff);
    }

    //--

    constexpr pattern_size_type bytes_count() const
    {
        return std::count(begin(), end(), ' ') + 1;
    }

    constexpr pattern_size_type segements_count() const
    {
        pattern_size_type count = 1;
        auto found              = false;

        for (auto c : buff)
        {
            if (c == '?')
            {
                found = true;
            }
            else if (found)
            {
                ++count;
                found = false;
            }
        }
        return count;
    }

    constexpr pattern_size_type longest_segment() const
    {
        pattern_size_type length         = 0;
        pattern_size_type current_length = 0;

        for (auto c : buff)
        {
            if (c == ' ')
            {
                ++current_length;
            }
            else if (c == '?' && length < current_length)
            {
                length         = current_length;
                current_length = 0;
            }
        }

        return length;
    }
};

template <size_t S>
pattern_string(char const (&)[S]) -> pattern_string<S - 1>;

struct pattern_segment_item
{
    uint8_t byte;
    bool unknown = true;

    template <std::same_as<uint8_t> T>
    constexpr operator T() const
    {
        assert(unknown == false);
        return byte;
    }
};

template <bool V>
inline constexpr auto pattern_segment_item_unknown = [](pattern_segment_item const& item) {
    return item.unknown == V;
};
} // namespace detail

template <detail::pattern_size_type BytesCount>
struct pattern_segment_bytes : array<uint8_t, BytesCount>
{
    template <typename It>
    constexpr pattern_segment_bytes(It known_from, It known_to)
        : array<uint8_t, BytesCount>()
    {
        assert(std::distance(known_from, known_to) == BytesCount);
        if constexpr (BytesCount == 1)
            this->front() = *known_from;
        else
            std::copy(known_from, known_to, this->begin());
    }
};

template <>
struct pattern_segment_bytes<0>;

template <>
struct pattern_segment_bytes<-1> : span<char const>
{
    using span::span;
};

template <detail::pattern_size_type BytesCount>
struct pattern_segment_unknown_bytes : std::integral_constant<detail::pattern_size_type, BytesCount>
{
    constexpr pattern_segment_unknown_bytes(std::integral_constant<detail::pattern_size_type, BytesCount> = {})
    {
    }
};

template <>
struct pattern_segment_unknown_bytes<-1>
{
    using value_type = detail::pattern_size_type;

    value_type value;

    template <detail::pattern_size_type BytesCount>
    constexpr pattern_segment_unknown_bytes(std::integral_constant<value_type, BytesCount>)
        : value{BytesCount}
    {
    }

    constexpr pattern_segment_unknown_bytes(value_type const bytes_count)
        : value{bytes_count}
    {
    }
};

template <detail::pattern_size_type Bytes, detail::pattern_size_type UnknownBytes>
struct pattern_segment
{
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto ubegin(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::known_bytes::const_pointer;
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto uend(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::known_bytes::const_pointer;
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto abs_size(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::unknown_bytes::value_type;

    using known_bytes   = pattern_segment_bytes<Bytes>;
    using unknown_bytes = pattern_segment_unknown_bytes<UnknownBytes>;

  private:
    known_bytes known_bytes_;
    [[no_unique_address]] //
    unknown_bytes unknown_bytes_;

  public:
    template <typename It>
    constexpr pattern_segment(It known_from, It known_to, unknown_bytes unknown_bytes = {})
        : known_bytes_(known_from, known_to)
        , unknown_bytes_(unknown_bytes)
    {
    }

    constexpr typename known_bytes::const_iterator begin() const
    {
        return known_bytes_.begin();
    }

    constexpr typename known_bytes::const_iterator end() const
    {
        return known_bytes_.begin();
    }

    constexpr typename unknown_bytes::value_type size() const
    {
        return known_bytes_.size();
    }
};

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto ubegin(pattern_segment<BytesCount, UnknownBytes> const& segment) ->
    typename pattern_segment<BytesCount, UnknownBytes>::known_bytes::const_pointer
{
    return ubegin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto uend(pattern_segment<BytesCount, UnknownBytes> const& segment) -> //
    typename pattern_segment<BytesCount, UnknownBytes>::known_bytes::const_pointer
{
    return ubegin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto abs_size(pattern_segment<BytesCount, UnknownBytes> const& segment) -> //
    typename pattern_segment<BytesCount, UnknownBytes>::unknown_bytes::value_type
{
    using std::size;
    return size(segment) + segment.unknown_bytes_.value;
}

template <class... Segment>
class pattern
{
    using storage_type = boost::hana::tuple<Segment...>;

    storage_type bytes_;

  public:
    constexpr pattern(Segment... segment)
        : bytes_(segment...)
    {
    }

    storage_type const& get() const
    {
        return bytes_;
    }

    detail::pattern_size_type size() const
    {
        detail::pattern_size_type sum = 0;
        boost::hana::for_each(bytes_, [&sum](auto& segment) {
            sum += abs_size(segment);
        });
        return sum;
    }
};

namespace detail
{
template <pattern_size_type BytesCount>
struct transformed_pattern : array<pattern_segment_item, BytesCount>
{
    constexpr transformed_pattern(span<char const> const pattern)
        : transformed_pattern()
    {
#ifdef _DEBUG
        auto splitted = std::views::split(pattern, ' ');
#else
        auto splitted = std::views::lazy_split(pattern, ' ');
#endif
        assert(std::ranges::distance(splitted) == BytesCount);

        std::ranges::for_each(splitted, [it = ubegin(*this)](auto&& byte) mutable {
            auto const first = ubegin(byte);
            auto const last  = uend(byte);

            auto const length = std::distance(first, last);
            assert(length == 1 || length == 2);

            if (*first == '?')
            {
                assert(length == 1 || *std::next(first) == '?');
                it->unknown = true;
            }
            else
            {
                auto const err = from_chars(first, first + length, it->byte, 16);
                assert(err.ec == std::errc{});
                it->unknown = false;
            }
            ++it;
        });
    }

    constexpr transformed_pattern() = default;

    constexpr size_t segments_count() const
    {
        size_t count = 0;

        auto const last = uend(*this);
        for (auto first = ubegin(*this); first != last; ++count)
        {
            first = std::find_if(first, last, pattern_segment_item_unknown<true>);
            first = std::find_if(first, last, pattern_segment_item_unknown<false>);
        }

        return count;
    }

    template <typename It>
    struct segment_view
    {
        using iterator = It; // pattern_segment_item const* /*typename array<pattern_segment_item, BytesCount>::const_iterator*/;

        iterator first;
        iterator last;
        pattern_size_type jump;

        /*constexpr pattern_size_type length() const
        {
            return std::distance(first, last);
        }*/
    };

    constexpr auto segment(size_t skip = 0) const -> segment_view<pattern_segment_item const*>
    {
        auto const first = ubegin(*this);
        auto const last  = uend(*this);

        for (auto it = first; it != last;)
        {
            auto const end  = std::find_if(it, last, pattern_segment_item_unknown<true>);
            auto const next = std::find_if(end, last, pattern_segment_item_unknown<false>);

            if (skip == 0)
                return {it, end, static_cast<pattern_size_type>(std::distance(end, next))};

            it = next;
            --skip;
        }

        return {first, last, skip == 0 ? 0 : BytesCount};
    }
};

// template <size_t SegmentLength>
// constexpr auto make_segment(auto tmp_segment)
//{
//     return pattern_segment<SegmentLength>(tmp_segment.begin, tmp_segment.end, tmp_segment.unknown);
// }

template <pattern_string Str>
struct static_pattern
{
    static constexpr auto bytes = transformed_pattern<Str.bytes_count()>({Str.begin(), Str.end()});
};

template <pattern_string Str>
constexpr auto make_pattern()
{
    constexpr auto bytes_count = Str.bytes_count();
    constexpr transformed_pattern<bytes_count> bytes({Str.begin(), Str.end()});

    auto make_segment = []<size_t Skip>(std::in_place_index_t<Skip>) {
        constexpr auto tmp_segment    = bytes.segment(Skip);
        constexpr auto segment_length = std::distance(tmp_segment.first, tmp_segment.last);
        return pattern_segment<segment_length, tmp_segment.jump>(tmp_segment.first, tmp_segment.last);
    };
    constexpr auto segments_count = bytes.segments_count();
    return [&]<size_t... I>(std::index_sequence<I...>) {
        return pattern{make_segment(std::in_place_index<I>)...};
    }(std::make_index_sequence<segments_count>());
}
} // namespace detail

inline namespace literals
{
template <detail::pattern_string Pattern>
constexpr auto operator""_pat()
{
#ifndef _DEBUG
    constexpr
#endif
        auto result = detail::make_pattern<Pattern>();
    return result;
}
} // namespace literals

template <typename... Args>
constexpr auto make_pattern(Args const&... args)
{
    constexpr auto segments_count = ((sizeof(Args) == sizeof(char) || std::constructible_from<span<char const>, Args const&>)+...);

    boost::hana::tuple<Args const&...> args_packed(args...);

    auto const make_segment = [&]<typename T>(T const& str, detail::pattern_size_type val) {
        using std::begin;
        using std::end;
        if constexpr (sizeof(T) == sizeof(char) && !std::is_class_v<T>)
            return pattern_segment<sizeof(T), -1>(&str, &str + 1, val);
        else
            return pattern_segment<-1, -1>(begin(str), end(str) - std::is_bounded_array_v<T>, val);
    };
    auto const make_segment_at = [&]<size_t I>(std::in_place_index_t<I>) {
        constexpr auto index = I % 2 ? I + 1 : I;
        auto const& str      = boost::hana::at_c<index>(args_packed);
        auto const num       = boost::hana::at_c<index + 1>(args_packed);
        return make_segment(str, num);
    };

    if constexpr (sizeof...(Args) % 2 == 0)
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment_at(std::in_place_index<I>)...};
        }(std::make_index_sequence<segments_count>());
    }
    else
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment_at(std::in_place_index<I>)..., make_segment(boost::hana::back(args_packed), 0)};
        }(std::make_index_sequence<segments_count - 1>());
    }
}
} // namespace fd
