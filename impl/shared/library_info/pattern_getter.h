#pragma once
#include "library_info/section_getter.h"
#include "pattern/find.h"
#ifdef __RESHARPER__
#include "pattern/basic_holder.h"
#endif

namespace fd
{
class library_info::basic_pattern_getter : public basic_object_getter
{
  public:
    template <class... Segment>
    void* find(pattern<Segment...> const& pat) const noexcept
    {
#if 0
        auto const first = linfo_->data();
        auto const last  = first + linfo_->size();
        auto const found = find_pattern(first, last, pat);
        if (found != last)
            return found;
#else
        auto const nt       = linfo_->nt_header();
        auto const img_base = safe_cast<uint8_t>(linfo_->image_base());

        auto first_section      = IMAGE_FIRST_SECTION(nt);
        auto const last_section = first_section + nt->FileHeader.NumberOfSections;

        for (; first_section != last_section; ++first_section)
        {
            auto const first = img_base + first_section->VirtualAddress;
            auto const last  = first + first_section->SizeOfRawData;

            auto const found = find_pattern(first, last, pat);
            if (found == last)
                continue;
            return found;
        }
#endif
        return nullptr;
    }
};
} // namespace fd