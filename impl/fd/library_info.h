#pragma once

#include <fd/magic_cast.h>
//
#include <fd/library_info/header.h>
#include <fd/library_info/library.h>
//
#include <fd/library_info/export.h>
#include <fd/library_info/game_interface.h>
#include <fd/library_info/pattern.h>
#include <fd/library_info/section.h>
#include <fd/library_info/vtable.h>

namespace fd
{
class library_info
{
    friend class game_library_info;
    friend class game_library_info_ex;

    LDR_DATA_TABLE_ENTRY *entry_;
    IMAGE_DOS_HEADER *dos_;
    IMAGE_NT_HEADERS *nt_;

  public:
    using auto_cast = from_void<auto_cast_tag>;

    template <size_t S>
    library_info(wchar_t const (&name)[S])
        : entry_(find_library(name, S - 1))
        , dos_(get_dos(entry_))
        , nt_(get_nt(dos_))
    {
    }

    template <size_t S>
    void *find_export(char const (&name)[S]) const
    {
        return fd::find_export(dos_, nt_, name, S - 1);
    }

    template <size_t S>
    auto_cast find_pattern(char const (&pattern)[S]) const
    {
        return fd::find_pattern(nt_, pattern, S - 1);
    }

    template <size_t S>
    IMAGE_SECTION_HEADER *find_section(char const (&name)[S]) const
    {
        return fd::find_section(nt_, name, S - 1);
    }
};

class game_library_info : public library_info
{
    game_interface *root_interface_;

  public:
    template <size_t S>
    game_library_info(wchar_t const (&name)[S])
        : library_info(name)
        , root_interface_(find_root_game_interface(find_export("CreateInterface")))
    {
    }

    template <size_t S>
    auto_cast find_interface(char const (&name)[S]) const
    {
        return find_game_interface(root_interface_, name, S - 1);
    }
};

class game_library_info_ex : public game_library_info
{
    IMAGE_SECTION_HEADER *rdata_;
    IMAGE_SECTION_HEADER *text_;

  public:
    template <size_t S>
    game_library_info_ex(wchar_t const (&name)[S])
        : game_library_info(name)
        , rdata_(find_section(".rdata"))
        , text_(find_section(".text"))
    {
    }

    template <size_t S>
    auto_cast find_vtable(char const (&name)[S]) const
    {
        return fd::find_vtable(rdata_, text_, find_rtti_descriptor(nt_, name, S - 1), name, S - 1);
    }
};

}