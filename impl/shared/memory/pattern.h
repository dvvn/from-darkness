#pragma once

#include "container/array.h"
#include "container/span.h"
#include "diagnostics/fatal.h"
#include "iterator/unwrap.h"
#include "memory/basic_pattern.h"
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
    uint8_t byte = -1;
    bool unknown = true;

    constexpr operator uint8_t() const
    {
        assert(!unknown);
        return byte;
    }
};

template <bool V>
inline constexpr auto pattern_segment_item_unknown = [](pattern_segment_item const& item) {
    return item.unknown == V;
};
} // namespace detail

template <class S>
class pattern_segment_base
{
    auto* bytes() const
    {
        return std::addressof(static_cast<S const*>(this)->known_bytes);
    }

    auto* bytes()
    {
        return std::addressof(static_cast<S*>(this)->known_bytes);
    }

  public:
    template <typename It>
    bool equal(It mem) const
    {
        return std::equal(bytes()->begin(), bytes()->end(), mem);
    }

    auto front() const
    {
        return bytes()->front();
    }

    auto begin() const
    {
        return bytes()->begin();
    }

    auto end() const
    {
        return bytes()->end();
    }

    auto size() const
    {
        return bytes()->size();
    }

    auto abs_size() const
    {
        return bytes()->size() + static_cast<S const*>(this)->unknown_bytes;
    }
};

template <detail::pattern_size_type BytesCount>
struct pattern_segment_bytes : array<uint8_t, BytesCount>
{
    template <typename It>
    constexpr pattern_segment_bytes(It known_from, It known_to)
        : array<uint8_t, BytesCount>()
    {
        assert(std::distance(known_from, known_to) == BytesCount);
        std::copy(known_from, known_to, this->begin());
    }
};

template <>
struct pattern_segment_bytes<1> : array<uint8_t, 1>
{
    template <typename It>
    constexpr pattern_segment_bytes(It known_from, It known_to)
        : array()
    {
        assert(std::distance(known_from, known_to) == 1);
        this->front() = *known_from;
    }
};

template <>
struct pattern_segment_bytes<0> : span<char const>
{
    using span::span;
};

template <detail::pattern_size_type BytesCount>
struct pattern_segment
{
    template <detail::pattern_size_type C>
    friend constexpr auto begin(pattern_segment<C> const&) -> typename pattern_segment<C>::storage_type::const_iterator;
    template <detail::pattern_size_type C>
    friend constexpr auto end(pattern_segment<C> const&) -> typename pattern_segment<C>::storage_type::const_iterator;
    template <detail::pattern_size_type C>
    friend constexpr auto ubegin(pattern_segment<C> const&) -> typename pattern_segment<C>::storage_type::const_pointer;
    template <detail::pattern_size_type C>
    friend constexpr auto uend(pattern_segment<C> const&) -> typename pattern_segment<C>::storage_type::const_pointer;
    template <detail::pattern_size_type C>
    friend constexpr auto size(pattern_segment<C> const&) -> typename pattern_segment<C>::size_type;
    template <detail::pattern_size_type C>
    friend constexpr auto abs_size(pattern_segment<C> const&) -> typename pattern_segment<C>::size_type;

    using storage_type = pattern_segment_bytes<BytesCount>;
    using size_type    = detail::pattern_size_type;

  private:
    storage_type known_bytes_;
    size_type unknown_bytes_;

  public:
    template <typename It>
    constexpr pattern_segment(It known_from, It known_to, size_type const unknown_bytes)
        : known_bytes_(known_from, known_to)
        , unknown_bytes_(unknown_bytes)
    {
    }
};

template <detail::pattern_size_type BytesCount>
constexpr auto begin(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::storage_type::const_iterator
{
    return begin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount>
constexpr auto end(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::storage_type::const_iterator
{
    return end(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount>
constexpr auto ubegin(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::storage_type::const_pointer
{
    return ubegin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount>
constexpr auto uend(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::storage_type::const_pointer
{
    return ubegin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount>
constexpr auto size(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::size_type
{
    if constexpr (BytesCount == 0)
        return segment.known_bytes_.size();
    else
        return BytesCount;
}

template <detail::pattern_size_type BytesCount>
constexpr auto abs_size(pattern_segment<BytesCount> const& segment) -> typename pattern_segment<BytesCount>::size_type
{
    return size(segment) + segment.unknown_bytes_;
}

template <detail::pattern_size_type... SegmentsBytesCount>
struct pattern
{
    boost::hana::tuple<pattern_segment<SegmentsBytesCount>...> bytes;

    constexpr pattern(pattern_segment<SegmentsBytesCount>... segment)
        : bytes(segment...)
    {
    }

    detail::pattern_size_type size() const
    {
        detail::pattern_size_type sum = 0;
        boost::hana::for_each(bytes, [&sum](auto& segment) {
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
                assert(length == 1 || *first == *std::next(first));
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

        iterator begin;
        iterator end;
        pattern_size_type unknown;

        constexpr pattern_size_type length() const
        {
            return std::distance(begin, end);
        }
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

template <size_t SegmentLength>
constexpr auto make_segment(auto tmp_segment)
{
    return pattern_segment<SegmentLength>(tmp_segment.begin, tmp_segment.end, tmp_segment.unknown);
}

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
        constexpr auto segment_length = tmp_segment.length();
        static_assert(segment_length != 0);
        return pattern_segment<segment_length>(tmp_segment.begin, tmp_segment.end, tmp_segment.unknown);
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
    constexpr auto segments_count = ((std::same_as<Args, char> || std::constructible_from<span<char const>, Args const&>)+...);

    boost::hana::tuple<Args const&...> args_packed(args...);

    auto const make_segment2 = [&]<typename T>(T const& str, auto unknown) {
        using std::begin;
        using std::end;
        if constexpr (std::is_bounded_array_v<T>)
            return pattern_segment<sizeof(T) - 1>(begin(str), end(str) - 1, unknown);
        else
            return pattern_segment<sizeof(T)>(begin(str), end(str), unknown);
    };

    auto const make_segment = [&]<size_t I>(std::in_place_index_t<I>) {
        constexpr auto index = I % 2 ? I + 1 : I;
        auto const& str      = boost::hana::at_c<index>(args_packed);
        auto const num       = boost::hana::at_c<index + 1>(args_packed);
        return make_segment2(str, num);
    };
    if constexpr (sizeof...(Args) % 2 == 0)
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment(std::in_place_index<I>)...};
        }(std::make_index_sequence<segments_count>());
    }
    else
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment(std::in_place_index<I>)..., make_segment2(boost::hana::back(args_packed), 0)};
        }(std::make_index_sequence<segments_count - 1>());
    }
}
} // namespace fd
