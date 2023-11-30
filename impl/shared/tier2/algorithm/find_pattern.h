#pragma once

#include "tier1/algorithm/find_pattern.h"
#include "tier2/algorithm/find.h"

namespace FD_TIER(2)
{
template <class Info = basic_library_info, typename Callback = find_callback_gap, class... Segment>
void* find(void* image_base, typename Info::sections_range sections, pattern<Segment...> const& pat, Callback callback = {})
{
    auto const& first_pattern_segment = boost::hana::front(pat.get());
    auto const first_pattern_byte     = *ubegin(first_pattern_segment);
    auto const callback_ref           = std::ref(callback);

    using std::size;

    if constexpr (sizeof...(Segment) == 1)
    {
        if (size(first_pattern_segment) == 1)
        {
            return detail::find_in_section(
                image_base, sections, //
                [=]<typename T>(T const u_section_start, T const u_section_end) -> T {
                    return find_one_byte(u_section_start, u_section_end, first_pattern_byte, callback);
                });
        }
    }

    return detail::find_in_section(
        image_base, sections, //
        [=, &pat, pat_size = size(pat)]<typename T>(T u_section_start, T const u_section_end) -> T {
            auto const u_section_end_safe = u_section_end - pat_size;

            using cb_invoker = find_callback_invoker<Callback, T>;

            if (u_section_start == u_section_end_safe)
            {
                if (!equal(u_section_start, pat))
                    return u_section_end;
                if (cb_invoker::call(callback_ref, u_section_start))
                    return u_section_start;
                return u_section_end;
            }

            if (u_section_start < u_section_end_safe)
                for (;;)
                {
                    auto const first_byte_found = std::find(u_section_start, u_section_end_safe, first_pattern_byte);
                    if (first_byte_found == u_section_end_safe)
                        break;
                    if (!equal(first_byte_found, pat))
                    {
                        u_section_start = first_byte_found + 1;
                        continue;
                    }
                    if (cb_invoker::call(callback_ref, first_byte_found))
                        return first_byte_found;
                    u_section_start = first_byte_found + pat_size;
                }

            return u_section_end;
        });
}

template <std::derived_from<basic_library_info> Info, typename Callback = find_callback_gap, class... Segment>
void* find(Info info, pattern<Segment...> const& pat, Callback callback = {})
{
    auto const nt_header = info.nt_header();
    return find(info.image_base(nt_header), typename Info::sections_range{nt_header}, pat, std::ref(callback));
}
} // namespace FD_TIER(2)