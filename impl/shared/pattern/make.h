#pragma once

#include "pattern/impl.h"
#include "string/static.h"

#include <boost/hana/back.hpp>

#include <charconv>

namespace fd
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

    constexpr transformed_pattern(span<char const> const pattern) // todo: raw iterators
        : buffer_{}
    {
#ifdef _DEBUG
        using std::ranges::distance;
        using std::views::split;
        assert(distance(split(pattern, ' ')) == BytesCount);
#endif
        auto it              = buffer_.data();
        auto byte0           = pattern.data();
        auto const last_byte = byte0 + pattern.size_bytes();

        auto const store_byte = [&]<size_t Num, size_t Offset>(std::in_place_index_t<Num>, std::in_place_index_t<Offset>) {
            if (*byte0 == '?')
            {
                if constexpr (Num != 1)
                    assert(*std::next(byte0) == '?');
                it->unknown = true;
            }
            else
            {
                auto const result = std::from_chars(byte0, byte0 + Num, it->byte, 16);
                assert(result.ec == std::errc{});
                it->unknown = false;
            }
            if constexpr (Offset != 0)
            {
                ++it;
                byte0 += Offset;
            }
        };

        for (;;)
        {
            auto const byte1 = byte0 + 1;
            if (byte1 == last_byte)
            {
                store_byte(std::in_place_index<1>, std::in_place_index<0>);
                break;
            }

            if (*byte1 == ' ')
            {
                store_byte(std::in_place_index<1>, std::in_place_index<2>);
                continue;
            }

            auto const byte2 = byte1 + 1;
            if (byte2 == last_byte)
            {
                store_byte(std::in_place_index<2>, std::in_place_index<0>);
                break;
            }

            store_byte(std::in_place_index<1>, std::in_place_index<3>);
        }

#if 0
        std::ranges::for_each(splitted, [it = buffer_.data()](auto const byte_rng) mutable {
            auto const byte_ptr = std::ranges::data(byte_rng);

            const auto store_byte = [=]<size_t Num>(std::in_place_index_t<Num>) {
                if (*byte_ptr == '?')
                {
                    if constexpr (Num == 2)
                        assert(*std::next(byte_ptr) == '?');
                    it->unknown = true;
                }
                else
                {
                    auto const err = std::from_chars(byte_ptr, byte_ptr + Num, it->byte, 16);
                    assert(err.ec == std::errc{});
                    it->unknown = false;
                }
            };

            switch (std::ranges::distance(byte_rng))
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
#endif
    }

    constexpr size_type segments_count() const
    {
        size_type count = 0;

        auto first      = buffer_.data();
        auto const last = first + buffer_.size();

        for (; first != last; ++count)
        {
            first = std::find_if(first, last, pattern_segment_item_unknown<true>);
            first = std::find_if(first, last, pattern_segment_item_unknown<false>);
        }

        return count;
    }

    struct segment_view
    {
        using iterator = pattern_segment_item const*;

        iterator first;
        iterator last;
        size_type jump;

        /*constexpr pattern_size_type length() const
        {
            return std::distance(first, last);
        }*/
    };

    constexpr segment_view segment(size_t skip = 0) const
    {
        auto const first = buffer_.data();
        auto const last  = first + buffer_.size();

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
    constexpr auto bytes_count = std::count(Str.data(), Str.data() + Str.length(), ' ') + 1;
    constexpr transformed_pattern<bytes_count> bytes{Str};

    constexpr auto make_segment = []<size_t Skip>(std::in_place_index_t<Skip>) {
        constexpr auto tmp_segment    = bytes.segment(Skip);
        constexpr auto segment_length = std::distance(tmp_segment.first, tmp_segment.last);
        return pattern_segment<segment_length, tmp_segment.jump>{tmp_segment.first};
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
            return pattern_segment<sizeof(T), -1>{&str, val};
        }
        else
        {
            using std::data;
            using std::size;
            auto const str_data = data(str);
            auto str_length     = size(str);
            if constexpr (std::is_bounded_array_v<T>)
                if (str_data[str_length - 1] == '\0')
                    --str_length;
            return pattern_segment<-1, -1>{
                {str_data, str_length},
                val
            };
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