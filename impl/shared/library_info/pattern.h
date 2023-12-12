#pragma once
#include "library_info/holder.h"
#include "pattern/find.h"

namespace fd::detail
{
template <typename Fn>
void* find_in_section(IMAGE_NT_HEADERS* const nt, uint8_t* const img_base, Fn fn)
{
    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    for (; first_section != last_section; ++first_section)
    {
        auto const first = img_base + first_section->VirtualAddress;
        auto const last  = first + first_section->SizeOfRawData;

        auto const found = fn(first, last);
        if (found == last)
            continue;
        return found;
    }

    return nullptr;
}

template <typename Fn>
void* find_in_section(IMAGE_NT_HEADERS* const nt, void* const img_base, Fn fn)
{
    using fn_pass = conditional_t<sizeof(Fn) <= sizeof(void*) * 2u && std::is_trivially_copyable_v<Fn>, Fn, std::add_lvalue_reference_t<Fn>>;
    return find_in_section<fn_pass>(nt, safe_cast<uint8_t>(img_base), fn);
}

template <typename Fn>
void* find_anywhere(uint8_t* const first, size_t const length, Fn fn)
{
    using std::data;
    using std::size;

    auto const last = first + length;

    auto found = fn(first, last);
    return found == last ? nullptr : found;
}

template <class... Segment>
void* library_pattern_getter<>::find(pattern<Segment...> const& pat) const
{
    // return find_anywhere(....);
    return find_in_section(linfo_->nt_header(), linfo_->image_base(), [&pat](uint8_t* const first, uint8_t* const last) -> void* {
        return find_pattern(first, last, pat);
    });
}
} // namespace fd::detail