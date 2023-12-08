#pragma once
#include "pattern/find.h"
#include "library_info.h"

namespace fd
{
template <typename Fn>
void* library_info::basic_pattern_getter::find_in_section(Fn fn) const
{
    auto const nt       = linfo_->nt_header();
    auto const img_base = safe_cast<uint8_t>(linfo_->image_base());

    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    for (; first_section != last_section; ++first_section)
    {
        auto first      = img_base + first_section->VirtualAddress;
        auto const last = first + first_section->SizeOfRawData;

        auto const found = fn(first, last);
        if (found == last)
            continue;
        return found;
    }

    return nullptr;
}

template <typename Fn>
void* library_info::basic_pattern_getter::find_anywhere(Fn fn) const
{
    auto mem_first      = linfo_->data();
    auto const mem_last = mem_first + linfo_->size();

    auto found = fn(mem_first, mem_last);
    return found == mem_last ? nullptr : found;
}

template <class... Segment>
void* library_info::basic_pattern_getter::find(pattern<Segment...> const& pat) const
{
    // return find_anywhere(....);
    return find_in_section([&pat]<typename T>(T const first, T const last) -> T {
        return find_pattern(first, last, pat);
    });
}
} // namespace fd