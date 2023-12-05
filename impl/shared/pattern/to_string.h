#pragma once

#include "algorithm/char.h"
#include "pattern.h"
#include "string/charconv.h"
#include "string/dynamic.h"

#include <boost/hana/for_each.hpp>

namespace fd
{
template <class... Segment>
constexpr auto to_string(pattern<Segment...> const& pat)
{
    using size_type     = pattern_size_type;
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
}