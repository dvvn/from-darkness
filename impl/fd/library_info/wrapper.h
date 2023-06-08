#pragma once

// #include "export.h"
// #include "game_interface.h"
// #include "header.h"
// #include "library.h"
// #include "pattern.h"
// #include "section.h"
// #include "vtable.h"

#include "core.h"

#include <string_view>

namespace fd
{
#define CONSTRUCTOR_ARRAY_NAME(_N_) \
    template <size_t S>             \
    _N_(wchar_t const(&name)[S])    \
        : _N_({name, S - 1})        \
    {                               \
    }

class library_info
{
    friend class game_library_info;
    friend class game_library_info_ex;

    LDR_DATA_TABLE_ENTRY *entry_;
    /*IMAGE_DOS_HEADER *dos_;
    IMAGE_NT_HEADERS *nt_;*/

  protected:
    LDR_DATA_TABLE_ENTRY *get() const;
    IMAGE_DOS_HEADER *dos() const;
    IMAGE_NT_HEADERS *nt() const;

  public:
    library_info(std::wstring_view name);
    CONSTRUCTOR_ARRAY_NAME(library_info);

    void *find_export(std::string_view name) const;

    template <typename Ret, typename... Args>
    auto find_export(std::string_view name) const -> Ret(__stdcall *)(Args...)
    {
        return static_cast<Ret(__stdcall *)(Args...)>(find_export(name));
    }

    void *find_pattern(std::string_view pattern) const;
    IMAGE_SECTION_HEADER *find_section(std::string_view name) const;
};
struct game_interface;

class game_library_info : public library_info
{
    game_interface *root_interface_;

  public:
    game_library_info(std::wstring_view name);
    CONSTRUCTOR_ARRAY_NAME(game_library_info);

    void *find_interface(std::string_view name) const;
};

class game_library_info_ex : public game_library_info
{
    IMAGE_SECTION_HEADER *rdata_;
    IMAGE_SECTION_HEADER *text_;

  public:
    game_library_info_ex(std::wstring_view name);
    CONSTRUCTOR_ARRAY_NAME(game_library_info_ex);

    void *find_vtable(std::string_view name) const;
};

#undef CONSTRUCTOR_ARRAY_NAME
} // namespace fd