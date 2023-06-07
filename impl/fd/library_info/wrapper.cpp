#include "export.h"
#include "game_interface.h"
#include "header.h"
#include "library.h"
#include "pattern.h"
#include "section.h"
#include "vtable.h"
#include "wrapper.h"

#include <algorithm>
#include <array>

namespace fd
{
LDR_DATA_TABLE_ENTRY *library_info::get() const
{
    return entry_;
}

IMAGE_DOS_HEADER *library_info::dos() const
{
    return get_dos(entry_);
}

IMAGE_NT_HEADERS *library_info::nt() const
{
    return get_nt(get_dos(entry_));
}

library_info::library_info(std::wstring_view name)
    : entry_(find_library(name.data(), name.length()))
{
}

void *library_info::find_export(std::string_view name) const
{
    using fd::find_export;
    auto dos = get_dos(entry_);
    auto nt  = get_nt(dos);
    return find_export(dos, nt, name.data(), name.length());
}

void * library_info::find_pattern(std::string_view pattern) const
{
    using fd::find_pattern;
    return find_pattern(nt(), pattern.data(), pattern.length());
}

IMAGE_SECTION_HEADER *library_info::find_section(std::string_view name) const
{
    using fd::find_section;
    return find_section(nt(), name.data(), name.length());
}

game_library_info::game_library_info(std::wstring_view name)
    : library_info(name)
    , root_interface_(find_root_game_interface(find_export("CreateInterface")))
{
}

from<void *> game_library_info::find_interface(std::string_view name) const
{
    return find_game_interface(root_interface_, name.data(), name.length(), false)->get();
}

template <size_t S>
static auto find_sections(IMAGE_NT_HEADERS *nt, std::string_view const (&names)[S])
{
    std::array<IMAGE_SECTION_HEADER *, S> ret;

    for (size_t i = 0; i != S; ++i)
        ret[i] = find_section(nt, names[i].data(), names[i].length());

    return ret;
}

template <size_t S>
static void find_sections(IMAGE_NT_HEADERS *nt, std::pair<IMAGE_SECTION_HEADER **, std::string_view> const (&names)[S])
{
    for (auto &p : names)
        *p.first = find_section(nt, p.second.data(), p.second.length());
}

game_library_info_ex::game_library_info_ex(std::wstring_view name)
    : game_library_info(name)
{
    find_sections(
        nt(),
        {
            {&rdata_, ".rdata"},
            { &text_,  ".text"}
    });
}

from<void *> game_library_info_ex::find_vtable(std::string_view name) const
{
    using fd::find_vtable;
    auto dos = get_dos(entry_);
    auto nt  = get_nt(dos);
    return find_vtable(rdata_, text_, dos, find_rtti_descriptor(nt, name.data(), name.length()));
}
}