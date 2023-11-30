#pragma once
#include "tier0/iterator/unwrap.h"
#include "tier1/algorithm/char.h"
#include "tier1/concepts.h"
#include "tier1/container/array.h"
#include "tier1/container/span.h"
#include "tier1/diagnostics/fatal.h"
#include "tier1/functional/bind.h"
#include "tier1/string/charconv.h"
#include "tier1/string/dynamic.h"
#include "tier1/string/static.h"

#include <boost/hana/append.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace FD_TIER(1)
{
namespace detail
{
#ifdef _DEBUG
using pattern_size_type       = size_t;
using pattern_difference_type = ptrdiff_t;
#else
using pattern_size_type       = uint8_t;
using pattern_difference_type = int8_t;
#endif

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

template <bool V>
inline constexpr auto pattern_segment_item_unknown = [](pattern_segment_item const& item) {
    return item.unknown == V;
};
} // namespace detail

template <detail::pattern_size_type BytesCount>
struct pattern_segment_bytes : array<uint8_t, BytesCount>
{
    using size_type = detail::pattern_size_type;

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

template <detail::pattern_size_type Bytes, detail::pattern_size_type UnknownBytes>
class pattern_segment
{
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto ubegin(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::known_pointer;
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto uend(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::known_pointer;
    template <detail::pattern_size_type Bytes1, detail::pattern_size_type UnknownBytes1>
    friend constexpr auto abs_size(pattern_segment<Bytes1, UnknownBytes1> const&) -> typename pattern_segment<Bytes1, UnknownBytes1>::size_type;

    using known_bytes_storage   = pattern_segment_bytes<Bytes>;
    using unknown_bytes_storage = pattern_segment_unknown_bytes<UnknownBytes>;

    known_bytes_storage known_bytes_;
    [[no_unique_address]] //
    unknown_bytes_storage unknown_bytes_;

  public:
    using size_type      = detail::pattern_size_type;
    using known_iterator = typename known_bytes_storage::const_iterator;
    using known_pointer  = typename known_bytes_storage::const_pointer;

    template <typename It>
    constexpr pattern_segment(It known_from, It known_to, unknown_bytes_storage unknown_bytes = {})
        : known_bytes_{known_from, known_to}
        , unknown_bytes_{unknown_bytes}
    {
    }

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

    constexpr span<typename known_bytes_storage::value_type const> get() const
    {
        return {known_bytes_};
    }

    constexpr size_type unknown() const
    {
        return unknown_bytes_.value;
    }
};

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto ubegin(pattern_segment<BytesCount, UnknownBytes> const& segment) -> typename pattern_segment<BytesCount, UnknownBytes>::known_pointer
{
    return ubegin(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto uend(pattern_segment<BytesCount, UnknownBytes> const& segment) -> typename pattern_segment<BytesCount, UnknownBytes>::known_pointer
{
    return uend(segment.known_bytes_);
}

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytes>
constexpr auto abs_size(pattern_segment<BytesCount, UnknownBytes> const& segment) -> typename pattern_segment<BytesCount, UnknownBytes>::size_type
{
    using std::size;
    return size(segment) + segment.unknown_bytes_.value;
}

template <class... Segment>
struct pattern
{
    using size_type = detail::pattern_size_type;

  private:
    using storage_type = boost::hana::tuple<Segment...>;

    storage_type bytes_;

  public:
    constexpr pattern(Segment... segment)
        : bytes_(segment...)
    {
    }

    constexpr storage_type const& get() const
    {
        return bytes_;
    }

    constexpr size_type size() const
    {
        size_type sum = 0;
        boost::hana::for_each(bytes_, [&sum](auto& segment) {
            sum += abs_size(segment);
        });
        return sum;
    }
};

namespace detail
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
} // namespace detail

template <class... Segment>
constexpr auto to_string(pattern<Segment...> const& pat)
{
    using size_type     = detail::pattern_size_type;
    constexpr auto impl = []<size_type Bytes, size_type UnknownBytes>(auto* str, pattern_segment<Bytes, UnknownBytes> const& segment) {
        char to_chars_buff[3];
        auto const to_char_buff_start = std::begin(to_chars_buff);

        const auto to_chars_helper = [&to_chars_buff, to_char_buff_start, to_chars_buff_end = std::end(to_chars_buff)] //
            <bool WriteSpace>(uint8_t const byte, std::bool_constant<WriteSpace>) {
                auto [buff_end, err] = std::to_chars(to_char_buff_start, std::end(to_chars_buff), byte, 16);
                assert(err == std::errc{});
                if (byte <= 0xF)
                {
                    to_chars_buff[1] = toupper(to_chars_buff[0]);
                    to_chars_buff[0] = '0';
                    ++buff_end;
                }
                if constexpr (WriteSpace)
                {
                    *buff_end = ' ';
                    ++buff_end;
                }

                return buff_end;
            };

        auto const segment_end = uend(segment);
        if (segment.unknown() == 0)
        {
            auto const pre_end = segment_end - 1;
            std::for_each(ubegin(segment), pre_end, [&](uint8_t const byte) {
                str->append(to_char_buff_start, to_chars_helper(byte, std::true_type()));
            });
            str->append(to_char_buff_start, to_chars_helper(*pre_end, std::false_type()));
        }
        else
        {
            std::for_each(ubegin(segment), segment_end, [&](uint8_t const byte) {
                str->append(to_char_buff_start, to_chars_helper(byte, std::true_type()));
            });
            constexpr char unknown_chars[] = {' ', '?'};
            for (auto i = segment.unknown(); i != 1; --i)
                str->append(std::begin(unknown_chars), std::end(unknown_chars));
            str->push_back(unknown_chars[1]);
        }
    };

    using detail::pattern_segment_constant_size;

    if constexpr ((complete<pattern_segment_constant_size<Segment>> && ...))
    {
        constexpr auto known_bytes_count   = ((pattern_segment_constant_size<Segment>::known) + ...);
        constexpr auto unknown_bytes_count = ((pattern_segment_constant_size<Segment>::unknown) + ...);

        constexpr auto chars_count_hex_max = known_bytes_count * 2;
        constexpr auto spaces_count        = known_bytes_count + unknown_bytes_count - 1;

        static_string<chars_count_hex_max + unknown_bytes_count + spaces_count> str;
        boost::hana::for_each(pat.get(), bind_front(impl, &str));
        return str;
    }
    else
    {
        string str;
        boost::hana::for_each(pat.get(), bind_front(impl, &str));
        str.shrink_to_fit();
        return str;
    }
}

namespace detail
{
template <pattern_size_type BytesCount>
struct transformed_pattern : array<pattern_segment_item, BytesCount>
{
    using size_type = pattern_size_type;

    constexpr transformed_pattern(span<char const> const pattern)
        : transformed_pattern()
    {
#ifdef _DEBUG
        auto splitted = std::views::split(pattern, ' ');
#else
        auto splitted = std::views::lazy_split(pattern, ' ');
#endif
        assert(std::ranges::distance(splitted) == BytesCount);

        using tier0::ubegin;
        using tier0::uend;

        std::ranges::for_each(splitted, [it = ubegin(*this)](auto const byte) mutable {
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

    constexpr transformed_pattern() = default;

    constexpr size_type segments_count() const
    {
        using tier0::ubegin;
        using tier0::uend;

        size_type count = 0;

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

        auto const first = ubegin(*this);
        auto const last  = uend(*this);

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
    constexpr auto bytes_count = std::count(Str.begin(), Str.end(), ' ') + 1;
    constexpr auto bytes       = transformed_pattern<bytes_count>({Str.begin(), Str.end()});

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
} // namespace FD_TIER(1)
