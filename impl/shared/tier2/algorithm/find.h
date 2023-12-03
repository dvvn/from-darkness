#pragma once

#include "tier0/algorithm/find.h"
#include "tier2/library_info/basic.h"

#include <cassert>

namespace FD_TIER(2)
{
template <class Info = basic_library_info, typename Fn>
void* find_in_section(void* image_base, typename Info::sections_range sections, Fn callback)
{
    
}

template <class Info = basic_library_info, typename It2, typename Callback = find_callback_gap>
void* find(void* image_base, typename Info::sections_range sections, It2 const what_first, It2 const what_last, Callback callback = {})
{
    verify_range(what_first, what_last);

    auto const u_what_first = unwrap_iterator(what_first);
    auto const u_what_last  = unwrap_iterator(what_last);

    auto const what_front    = *u_what_first;
    auto const target_length = std::distance(u_what_first, u_what_last);

    auto const callback_ref = std::ref(callback);

    if (target_length == 1)
    {
        return find_in_section(image_base, sections, [=]<typename T>(T const u_section_first, T const u_section_last) -> T {
            return find_byte<true>(u_section_first, u_section_last, what_front, callback_ref);
        });
    }

    return find_in_section(image_base, sections, [=]<typename T>(T u_section_first, T const u_section_last) -> T {
        auto const u_section_last_safe = u_section_last - target_length;

        for (;;)
        {
            auto const u_section_what_front = std::find(u_section_first, u_section_last_safe, what_front);
            if (u_section_what_front == u_section_last_safe)
                return u_section_last;
            if (!std::equal(u_what_first, u_what_last, u_section_what_front))
            {
                u_section_first = u_section_first + 1;
                continue;
            }
            if (!invoke_find_callback(callback, u_section_what_front))
            {
                u_section_first = u_section_what_front + target_length;
                continue;
            }
            return u_section_what_front;
        }
    });
}

template <class Info = basic_library_info>
auto find(typename Info::sections_range const sections, string_view name) -> typename Info::sections_range::const_iterator
{
    
}
}