#pragma once

#include "pattern/holder.h"
#include "string/static.h"
#include "type_traits/integral_constant_literal.h"

#include <boost/hana/back.hpp>

#include <charconv>
#ifdef _DEBUG
#include <ranges>
#endif

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

template <size_t BytesCount>
class transformed_pattern
{
    using buffer_type = array<pattern_segment_item, BytesCount>;

    buffer_type buffer_;

    template <bool V>
    static constexpr auto pattern_segment_item_unknown(pattern_segment_item const& item)
    {
        return item.unknown == V;
    }

  public:
#ifdef FD_PATTERN_NATIVE_SIZE
    using size_type = size_t;
#else
    using size_type = small_type<size_t, BytesCount>;
#endif

    template <class T>
    constexpr transformed_pattern(T const& pat)
        : buffer_{}
    {
        using std::data;
        using std::size;

        auto it              = data(buffer_);
        auto byte0           = data(pat);
        auto const last_byte = byte0 + size(pat);

        // resharper ignore 'typename N'
#ifdef __RESHARPER__
        // ReSharper disable once CppInconsistentNaming
        using N = size_t;
#endif
        auto const store_byte = [&]<typename N, N Num, N Offset>(integral_constant<N, Num>, integral_constant<N, Offset>) {
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
                store_byte(1_c, 0_c);
                break;
            }

            if (*byte1 == ' ')
            {
                store_byte(1_c, 2_c);
                continue;
            }

            auto const byte2 = byte1 + 1;
            if (byte2 == last_byte)
            {
                store_byte(2_c, 0_c);
                break;
            }

            store_byte(2_c, 3_c);
        }
    }

    constexpr size_type segments_count() const
    {
        size_type count = 0;

#ifdef _DEBUG
        auto first      = buffer_.data();
        auto const last = first + buffer_.size();
#else
        auto first      = buffer_.begin();
        auto const last = buffer_.end();
#endif

        for (; first != last; ++count)
        {
            first = std::find_if(first, last, pattern_segment_item_unknown<true>);
            first = std::find_if(first, last, pattern_segment_item_unknown<false>);
        }

        return count;
    }

    struct segment_view
    {
#ifdef _DEBUG
        using iterator = pattern_segment_item const*;
#else
        using iterator = buffer_type::const_iterator;
#endif
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
#ifdef _DEBUG
        auto const first = buffer_.data();
        auto const last  = first + buffer_.size();
#else
        auto const first = buffer_.begin();
        auto const last  = buffer_.end();
#endif
        for (auto it = first; it != last;)
        {
            auto const unk_start = std::find_if(it, last, pattern_segment_item_unknown<true>);
            auto const unk_end   = std::find_if(unk_start, last, pattern_segment_item_unknown<false>);

            if (skip == 0)
                return {it, unk_start, static_cast<size_type>(std::distance(unk_start, unk_end))};

            it = unk_end;
            --skip;
        }

        return {first, last, skip == 0 ? 0u : BytesCount};
    }
};

template <auto Str>
constexpr auto make_pattern()
{
    constexpr auto bytes_count = []() -> size_t {
#ifdef _DEBUG
        auto const first = Str.data();
        auto const last  = first + Str.size();
#else
        auto const first = buffer_.begin();
        auto const last  = buffer_.end();
#endif
        return std::count(first, last, ' ');
    }() + 1;

    constexpr transformed_pattern<bytes_count> bytes{Str};

    constexpr auto make_segment = []<size_t Skip>(integral_constant<size_t, Skip>) {
        constexpr auto tmp_segment    = bytes.segment(Skip);
        constexpr auto segment_length = std::distance(tmp_segment.first, tmp_segment.last);
        return pattern_segment<segment_length, tmp_segment.jump>{tmp_segment.first};
    };
    constexpr auto segments_count = bytes.segments_count();
    return [&]<size_t... Index>(std::index_sequence<Index...>) {
        return pattern{make_segment(integral_constant<size_t, Index>{})...};
    }(std::make_index_sequence<segments_count>{});
}
} // namespace detail

inline namespace literals
{
#ifdef _DEBUG
template <basic_constant_string Pattern>
constexpr auto operator""_pat()
{
    auto result = detail::make_pattern<Pattern>();
    return result;
}
#else
template <constant_string Pattern>
consteval auto operator""_pat()
{
    constexpr auto result = detail::make_pattern<Pattern>();
    return result;
}
#endif

} // namespace literals

template <typename... Args>
constexpr auto make_pattern(Args const&... args)
{
    constexpr auto segments_count = ((sizeof(Args) == sizeof(char) || std::constructible_from<span<char const>, Args const&>)+...);

    boost::hana::tuple<Args const&...> args_packed{args...};

    auto const make_segment = [&]<typename T>(T const& str, size_t val) {
        if constexpr (sizeof(T) == sizeof(char) && !std::is_class_v<T>)
            return pattern_segment<sizeof(T), -1>{&str, val};
        else
            return pattern_segment<-1, -1>{str, val};
    };
    auto const make_segment_at = [&]<size_t I>(integral_constant<size_t, I>) {
        constexpr auto index = I % 2 ? I + 1 : I;
        auto const& str      = boost::hana::at_c<index>(args_packed);
        auto const num       = boost::hana::at_c<index + 1>(args_packed);
        return make_segment(str, num);
    };

    if constexpr (sizeof...(Args) % 2 == 0)
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment_at(integral_constant<size_t, I>{})...};
        }(std::make_index_sequence<segments_count>());
    }
    else
    {
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return pattern{make_segment_at(integral_constant<size_t, I>{})..., make_segment(boost::hana::back(args_packed), 0)};
        }(std::make_index_sequence<segments_count - 1>());
    }
}
} // namespace fd