#pragma once

#include "algorithm/find.h"
#include "memory/pattern_fwd.h"

#include <boost/hana/front.hpp>
#include <boost/hana/unpack.hpp>

namespace fd
{
template <typename It, detail::pattern_size_type Bytes, detail::pattern_size_type UnknownBytes, typename... Next>
bool equal(It mem, pattern_segment<Bytes, UnknownBytes> const& segment, Next const&... next)
{
    using std::begin;
    using std::end;

    auto const ok = std::equal(begin(segment), end(segment), mem);

    if constexpr (sizeof...(Next) == 0)
        return ok;
    else
        return ok && equal(mem + abs_size(segment), next...);
}

template <typename It, class... Segment>
bool equal(It u_start, pattern<Segment...> const& pat)
{
    return boost::hana::unpack(pat.get(), [u_start](auto&... segment) -> bool {
        return equal(u_start, segment...);
    });
}

template <typename Callback = uint8_t, typename It, class... Segment>
It find(It rng_start, It const rng_end, pattern<Segment...> const& pat, Callback callback = {})
{
    auto const first_pattern_byte = *ubegin(boost::hana::front(pat.get()));
    auto const callback_ref       = std::ref(callback);

    if constexpr (sizeof...(Segment) == 1)
    {
        return detail::find_one_byte(rng_start, rng_end, first_pattern_byte, callback_ref);
    }
    else
    {
        using std::size;

        verify_range(rng_start, rng_end);

        auto const pat_size = size(pat);

        auto u_rng_start          = unwrap_iterator(rng_start);
        auto const u_rng_end_safe = unwrap_iterator(rng_end - pat_size);

        verify_range(u_rng_start, u_rng_end_safe);

        for (;;)
        {
            auto const first_byte_found = std::find(u_rng_start, u_rng_end_safe, first_pattern_byte);
            if (first_byte_found == u_rng_end_safe)
                return rng_end;
            if (!equal(first_byte_found, pat))
                ++u_rng_start;
            else if (detail::on_found_byte(rng_start, first_byte_found, callback_ref))
                return rng_start;
            u_rng_start = first_byte_found + pat_size;
        }
    }
}

// template <typename Callback = uint8_t, class ...Segment>
// void* find(void* first, void const* last, pattern<Segment...> const& pat, Callback callback = {})
//{
//     return find(static_cast<uint8_t*>(first), static_cast<uint8_t const*>(last), pat, std::ref(callback));
// }

class basic_library_info;
struct library_section_view;
struct library_sections_range;

namespace detail
{
library_sections_range make_sections_range(void* nt);
library_section_view make_section_view(void const* section, void* image_base);
} // namespace detail

template <typename Callback = uint8_t, std::derived_from<basic_library_info> Info, class... Segment>
void* find(Info info, pattern<Segment...> const& pat, Callback callback = {})
{
    auto const nt_header  = info.nt_header();
    auto const image_base = info.image_base(nt_header);
    auto const sections   = detail::make_sections_range(nt_header);

    auto const first_pattern_byte = *ubegin(boost::hana::front(pat.get()));
    auto const callback_ref       = std::ref(callback);

    auto first_section      = ubegin(sections);
    auto const last_section = uend(sections);

    if constexpr (sizeof...(Segment) == 1)
    {
        for (; first_section != last_section; ++first_section)
        {
            auto const section_view = detail::make_section_view(first_section, image_base);
            auto const section_end  = uend(section_view);

            auto const found = detail::find_one_byte<false>(ubegin(section_view), section_end, first_pattern_byte, callback_ref);
            if (found != section_end)
                return found;
        }
    }
    else
    {
        using std::size;

        auto const pat_size = size(pat);

        for (; first_section != last_section; ++first_section)
        {
            auto const section_view = detail::make_section_view(first_section, image_base);

            auto section_start          = ubegin(section_view);
            auto const section_end_safe = uend(section_view) - pat_size;

            if (section_start <= section_end_safe)
                continue;

            for (;;)
            {
                auto const first_byte_found = std::find(section_start, section_end_safe, first_pattern_byte);
                if (first_byte_found == section_end_safe)
                    break;
                if (!equal(first_byte_found, pat))
                    ++section_start;
                else if (detail::on_found_byte(section_start, first_byte_found, callback_ref))
                    return section_start;
                section_start = first_byte_found + pat_size;
            }
        }
    }

    return nullptr;
}
}