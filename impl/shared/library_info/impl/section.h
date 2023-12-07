#pragma once

namespace fd
{
inline auto library_info::basic_section_getter::find(string_view name) const -> pointer
{
    auto const name_length = name.length();

    assert(name_length < sizeof(IMAGE_SECTION_HEADER::Name));

    auto const nt = linfo_->nt_header();

    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    auto const name_first = name.data();
    auto const name_last  = name_first + name.length();

    for (; first_section != last_section; ++first_section)
    {
        if (first_section->Name[name_length] != '\0')
            continue;
        if (!std::equal(name_first, name_last, first_section->Name))
            continue;
        return first_section;
    }

    return nullptr;
}
}