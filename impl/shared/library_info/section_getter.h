#pragma once

#include "library_info/holder.h"

#include <algorithm>
#include <cassert>

template <typename V, class C, size_t Length>
constexpr size_t size(V (C::*)[Length]) noexcept
{
    return Length;
}

namespace fd::detail
{
class library_section_getter
{
  public:
    using pointer = IMAGE_SECTION_HEADER const*;

  private:
    pointer first_;
    pointer last_;

  public:
    library_section_getter(library_info const* linfo)
    {
        auto const nt = linfo->nt_header();

        first_ = IMAGE_FIRST_SECTION(nt);
        last_  = first_ + nt->FileHeader.NumberOfSections;
    }

    pointer begin() const
    {
        return first_;
    }

    pointer end() const
    {
        return last_;
    }

    pointer find(string_view const name) const noexcept
    {
        using std::data;
        using std::size;

        auto const name_length = size(name);

        assert(name_length < size(&IMAGE_SECTION_HEADER::Name));

        auto const name_first = data(name);
        auto const name_last  = name_first + name_length;

        for (auto it = begin(); it != end(); ++it)
        {
            using std::equal;

            if (it->Name[name_length] != '\0')
                continue;
            if (!equal(name_first, name_last, it->Name))
                continue;
            return it;
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

class library_section_getter_ex : public library_section_getter
{
    uint8_t* image_base_;

  public:
    library_section_getter_ex(library_info const* linfo)
        : library_section_getter{linfo}
        , image_base_{safe_cast_from(linfo->image_base())}
    {
    }

    using library_section_getter::begin;
    using library_section_getter::end;

    uint8_t* begin(pointer const section) const
    {
        return image_base_ + section->VirtualAddress;
    }

    uint8_t* end(pointer const section) const
    {
        return begin(section) + section->SizeOfRawData;
    }
};
} // namespace fd::detail