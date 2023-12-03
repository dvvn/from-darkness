#pragma once

#include "tier1/pattern.h"
#include "tier1/string/charconv.h"
#include "tier1/string/static.h"

#include <boost/hana/back.hpp>

namespace FD_TIER(1)
{
namespace detail
{
struct pattern_segment_item
{
    uint8_t byte;
    bool unknown;

    constexpr pattern_segment_item()
    {
        if (std::is_constant_evaluated())
        {
            byte    = -1;
            unknown = true;
        }
    }

    template <std::same_as<uint8_t> T>
    constexpr operator T() const
    {
        assert(unknown == false);
        return byte;
    }
};

template <pattern_size_type BytesCount>
class transformed_pattern
{
    array<pattern_segment_item, BytesCount> buffer_;

    template <bool V>
    static constexpr auto pattern_segment_item_unknown(pattern_segment_item const& item)
    {
        return item.unknown == V;
    }

  public:
    using size_type = pattern_size_type;

    constexpr transformed_pattern(span<char const> const pattern)
        : buffer_{}
    {
#ifdef _DEBUG
        auto splitted = std::views::split(pattern, ' ');
#else
        auto splitted = std::views::lazy_split(pattern, ' ');
#endif
        assert(std::ranges::distance(splitted) == BytesCount);

        std::ranges::for_each(splitted, [it = ubegin(buffer_)](auto const byte) mutable {
            auto const first_byte = ubegin(byte);
            auto const last_byte  = uend(byte);

            const auto store_byte = [=]<size_t Num>(std::in_place_index_t<Num>) {
                if (*first_byte == '?')
                {
                    if constexpr (Num == 2)
                        assert(*std::next(first_byte) == '?');
                    it->unknown = true;
                }
                else
                {
                    auto const err = from_chars(first_byte, last_byte, it->byte, 16);
                    assert(err.ec == std::errc{});
                    it->unknown = false;
                }
            };

            switch (std::distance(first_byte, last_byte))
            {
            case 1:
                store_byte(std::in_place_index<1>);
                break;
            case 2:
                store_byte(std::in_place_index<2>);
                break;
            default:
                unreachable();
            }
            ++it;
        });
    }

    constexpr size_type segments_count() const
    {
        size_type count = 0;

        auto const last = uend(buffer_);
        for (auto first = ubegin(buffer_); first != last; ++count)
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
        size_type jump;

        /*constexpr pattern_size_type length() const
        {
            return std::distance(first, last);
        }*/
    };

    constexpr auto segment(size_t skip = 0) const -> segment_view<pattern_segment_item const*>
    {
        using tier0::ubegin;
        using tier0::uend;

        auto const first = ubegin(buffer_);
        auto const last  = uend(buffer_);

        for (auto it = first; it != last;)
        {
            auto const end  = std::find_if(it, last, pattern_segment_item_unknown<true>);
            auto const next = std::find_if(end, last, pattern_segment_item_unknown<false>);

            if (skip == 0)
                return {it, end, static_cast<size_type>(std::distance(end, next))};

            it = next;
            --skip;
        }

        return {first, last, skip == 0 ? 0 : BytesCount};
    }
};

template <static_string Str>
constexpr auto make_pattern()
{
    constexpr auto bytes_count = std::count(ubegin(Str), uend(Str), ' ') + 1;
    constexpr transformed_pattern<bytes_count> bytes{Str};

    constexpr auto make_segment = []<size_t Skip>(std::in_place_index_t<Skip>) {
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
template <static_string Pattern>
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

    auto const make_segment = [&]<typename T>(T const& str, pattern_size_type val) {
        if constexpr (sizeof(T) == sizeof(char) && !std::is_class_v<T>)
        {
            return pattern_segment<sizeof(T), -1>{&str, &str + 1, val};
        }
        else
        {
            using tier0::ubegin;
            using tier0::uend;
            auto str_last = uend(str);
            if constexpr (std::is_bounded_array_v<T>)
                if (*(str_last - 1) == '\0')
                    --str_last;
            return pattern_segment<-1, -1>{ubegin(str), str_last, val};
        }
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
}