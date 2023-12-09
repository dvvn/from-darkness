#pragma once

#include "algorithm/char.h"
#include "pattern/holder.h"
#include "string/dynamic.h"
#include "string/static.h"

#include <cassert>
#include <charconv>

namespace fd
{
namespace detail
{
class patter_to_string_info
{
    static constexpr char unknown_chars[] = {'?', ' '};

  public:
    static constexpr size_t predict_size(size_t const known, size_t const unknown)
    {
        auto const chars_count_hex_max = known * 2;
        auto const spaces_count        = known + unknown - 1;
        auto const unknown_chars_count = unknown * (size(unknown_chars) - 1);

        return chars_count_hex_max + spaces_count + unknown_chars_count;
    }

    template <bool LastSegment, size_t Bytes, size_t UnknownBytes>
    static constexpr void append(pattern_segment<Bytes, UnknownBytes> const& segment, auto& out_str)
#ifdef _DEBUG
        requires requires { out_str.push_back(0); }
#endif
    {
        char to_chars_buff[3];

        auto const to_char_buff_start = std::begin(to_chars_buff);

        auto const to_chars_helper = [&to_chars_buff, to_char_buff_start, to_chars_buff_end = std::end(to_chars_buff)] //
            <bool WriteSpace>(uint8_t const byte, bool_constant<WriteSpace>) {
                auto [buff_end, err] = std::to_chars(to_char_buff_start, to_chars_buff_end, byte, 16);
#ifdef _DEBUG
                assert(err == std::errc{});
                if (!std::is_constant_evaluated())
                    std::fill(buff_end, to_chars_buff_end, 0);
#endif
                if (byte > 0xF)
                {
                    to_chars_buff[1] = toupper(to_chars_buff[1]);
                    to_chars_buff[0] = toupper(to_chars_buff[0]);
                }
                else
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

        auto const& segment_bytes = segment.get();
        auto const segment_first  = segment_bytes.data();
        auto const segment_length = segment_bytes.size();
        if (segment.unknown() == 0)
        {
            auto const segment_pre_last = segment_first + segment_length - 1;
            std::for_each(segment_first, segment_pre_last, [&](uint8_t const byte) {
                out_str.append(to_char_buff_start, to_chars_helper(byte, true_type{}));
            });
            out_str.append(to_char_buff_start, to_chars_helper(*segment_pre_last, bool_constant<LastSegment == false>{}));
        }
        else
        {
            std::for_each(segment_first, segment_first + segment_length, [&](uint8_t const byte) {
                out_str.append(to_char_buff_start, to_chars_helper(byte, true_type{}));
            });
            if constexpr (LastSegment)
            {
                for (size_t i = segment.unknown(); i != 1; --i)
                    out_str.append(unknown_chars, size(unknown_chars));
                out_str.append(unknown_chars, size(unknown_chars) - 1);
            }
            else
            {
                for (size_t i = segment.unknown(); i != 0; --i)
                    out_str.append(unknown_chars, size(unknown_chars));
            }
        }
    }
};
} // namespace detail

template <class... Segment>
constexpr auto to_string(pattern<Segment...> const& pat)
{
    using info = detail::patter_to_string_info;

    auto const call_impl = [&](auto& out_str) {
        [&out_str, &pat]<size_t... I>(std::integer_sequence<size_t, I...> seq) {
            constexpr auto last_index = seq.size() - 1;
            auto const& pat_raw       = pat.get();
            (info::append<I == last_index>(boost::hana::at_c<I>(pat_raw), out_str), ...);
        }(std::make_integer_sequence<size_t, sizeof...(Segment)>());
    };

    if constexpr ((complete<pattern_segment_constant_size<Segment>> && ...))
    {
        constexpr auto known_bytes_count   = ((pattern_segment_constant_size<Segment>::known) + ...);
        constexpr auto unknown_bytes_count = ((pattern_segment_constant_size<Segment>::unknown) + ...);

        static_string<info::predict_size(known_bytes_count, unknown_bytes_count)> out_str;
        call_impl(out_str);
        return out_str;
    }
    else
    {
        string out_str;

        if (!std::is_constant_evaluated())
        {
            size_t known_bytes_count, unknown_bytes_count;

            boost::hana::unpack(pat.get(), [&](auto&... segment) {
                known_bytes_count   = (static_cast<size_t>(segment.known()) + ...);
                unknown_bytes_count = (static_cast<size_t>(segment.unknown()) + ...);
            });

            out_str.reserve(info::predict_size(known_bytes_count, unknown_bytes_count));
        }

        call_impl(out_str);
        return std::move(out_str);
    }
}
} // namespace fd