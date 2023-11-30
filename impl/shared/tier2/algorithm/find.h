#pragma once

#include "tier0/algorithm/find.h"
#include "tier2/library_info/basic.h"

#include <cassert>

namespace FD_TIER(2)
{
namespace detail
{
template <class Info = basic_library_info, typename Fn>
void* find_in_section(void* image_base, typename Info::sections_range sections, Fn callback)
{
    auto first_section      = ubegin(sections);
    auto const last_section = uend(sections);

    for (; first_section != last_section; ++first_section)
    {
        auto [u_section_start, u_section_end] = unwrap_section<Info>(first_section, image_base);
        auto const found                      = callback(u_section_start, u_section_end);
        if (found == u_section_end)
            continue;
        return found;
    }

    return nullptr;
}
} // namespace detail

template <class Info = basic_library_info, typename It2, typename Callback = find_callback_gap>
void* find(void* image_base, typename Info::sections_range sections, It2 const what_start, It2 const what_end, Callback callback = {})
{
    verify_range(what_start, what_end);

    auto const u_what_start = unwrap_iterator(what_start);
    auto const u_what_end   = unwrap_iterator(what_end);

    auto const what_front    = *u_what_start;
    auto const target_length = std::distance(u_what_start, u_what_end);

    auto const callback_ref = std::ref(callback);

    if (target_length == 1)
    {
        return detail::find_in_section(
            image_base, sections, //
            [=]<typename T>(T const u_section_start, T const u_section_end) -> T {
                return find_one_byte(u_section_start, u_section_end, what_front, callback_ref);
            });
    }

    return detail::find_in_section(
        image_base, sections, //
        [=]<typename T>(T u_section_start, T const u_section_end) -> T {
            auto const u_section_end_safe = u_section_end - target_length;

            using cb_invoker = find_callback_invoker<Callback, T>;

            for (;;)
            {
                auto const front_found = std::find(u_section_start, u_section_end_safe, what_front);
                if (front_found == u_section_end_safe)
                    return u_section_end;
                if (!std::equal(u_what_start, u_what_end, u_section_start))
                {
                    u_section_start = u_section_start + 1;
                    continue;
                }
                if (cb_invoker::call(callback_ref, u_section_start))
                    return u_section_start;

                u_section_start = front_found + target_length;
            }
        });
}

template <class Info = basic_library_info>
auto find(typename Info::sections_range const sections, string_view name) -> typename Info::sections_range::const_pointer
{
    auto const name_length = name.length();

    assert(name_length < sizeof(IMAGE_SECTION_HEADER::Name));

    auto u_first_header      = ubegin(sections);
    auto const u_last_header = ubegin(sections);

    for (; u_first_header != u_last_header; ++u_first_header)
    {
        if (u_first_header->Name[name_length] != '\0')
            continue;

        if (!std::equal(u_first_header->Name, u_first_header->Name + name_length, ubegin(name), uend(name)))
            continue;

        break;
    }

    return u_first_header;
}
}