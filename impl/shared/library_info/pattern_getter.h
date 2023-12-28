#pragma once
#include "library_info/section_getter.h"
#include "pattern/find.h"
#ifdef __RESHARPER__
#include "pattern/basic_holder.h"
#endif

namespace fd
{
namespace detail
{
template <typename Fn>
void* find_in_section(IMAGE_NT_HEADERS* const nt, uint8_t* const img_base, Fn fn) noexcept
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
void* find_anywhere(uint8_t* const first, size_t const length, Fn fn) noexcept
{
    using std::data;
    using std::size;

    auto const last = first + length;

    auto found = fn(first, last);
    return found == last ? nullptr : found;
}
} // namespace detail

struct library_info::basic_pattern_getter : basic_section_getter
{
    template <class... Segment>
    void* find(pattern<Segment...> const& pat) const noexcept
    {
        // return find_anywhere(....);
        return detail::find_in_section(
            linfo_->nt_header(), safe_cast<uint8_t>(linfo_->image_base()), //
            [&pat](uint8_t* const first, uint8_t* const last) -> void* {
                return find_pattern(first, last, pat);
            });
    }
};

} // namespace fd