#pragma once

#include "library_info/holder.h"

#include <algorithm>
#include <cassert>

namespace fd
{
class library_info::basic_section_getter : public basic_object_getter
{
  public:
    using pointer = IMAGE_SECTION_HEADER const*;

    pointer find(string_view const name) const noexcept
    {
        using std::data;
        using std::equal;
        using std::size;

        auto const nt = linfo_->nt_header();

        auto first_section      = IMAGE_FIRST_SECTION(nt);
        auto const last_section = first_section + nt->FileHeader.NumberOfSections;

        auto const name_first  = data(name);
        auto const name_length = size(name);
        auto const name_last   = name_first + name_length;

        assert(name_length < size(first_section->Name));

        for (; first_section != last_section; ++first_section)
        {
            if (first_section->Name[name_length] != '\0')
                continue;
            if (!equal(name_first, name_last, first_section->Name))
                continue;
            return first_section;
        }

        return nullptr;
    }

    pointer rdata() const
    {
        return find(".rdata");
    }

    pointer text() const
    {
        return find(".text");
    }
};
} // namespace fd