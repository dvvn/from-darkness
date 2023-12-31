#pragma once
#include "library_info/section_getter.h"
#include "pattern/find.h"
#ifdef __RESHARPER__
#include "pattern/basic_holder.h"
#endif

namespace fd::detail
{
class library_pattern_getter
{
#if 0

#else
    library_section_getter_ex
#endif
        section_;

  public:
    library_pattern_getter(library_info const* linfo)
        : section_{linfo}
    {
    }

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
        for (auto section = section_.begin(); section != section_.end(); ++section)
        {
            auto const first = section_.get_begin(section);
            auto const last  = section_.get_end(section);

            auto const found = find_pattern(first, last, pat);
            if (found == last)
                continue;
            return found;
        }
#endif
        return nullptr;
    }
};
} // namespace fd::detail