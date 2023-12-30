#pragma once

#include "library_info/holder.h"

#include <algorithm>

namespace fd::detail
{
class library_function_getter
{
    uint8_t* base_address;
    uint32_t* names;
    uint32_t* funcs;
    uint16_t* ords;
    DWORD last_offset;

  public:
    library_function_getter(library_info const* linfo)
    {
        auto const nt         = linfo->nt_header();
        base_address          = linfo->data();
        auto const export_dir = unsafe_cast<IMAGE_EXPORT_DIRECTORY*>(
            base_address + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        // auto const export_dir_end = export_dir_start+entry_export.Size;
        names = unsafe_cast_from(base_address + export_dir->AddressOfNames);
        funcs = unsafe_cast_from(base_address + export_dir->AddressOfFunctions);
        ords  = unsafe_cast_from(base_address + export_dir->AddressOfNameOrdinals);

        last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    }

    void* find(string_view const name) const noexcept
    {
        auto const name_first  = name.data();
        auto const name_length = name.length();
        auto const name_last   = name_first + name_length;

        for (DWORD offset = 0; offset != last_offset; ++offset)
        {
            using std::equal;

            auto const fn_name_raw = unsafe_cast<char const*>(base_address + names[offset]);
            if (fn_name_raw[name_length] != '\0')
                continue;
            if (!equal(name_first, name_last, fn_name_raw))
                continue;

            auto const fn = base_address + funcs[ords[offset]];
            // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
            return unsafe_cast_from(fn);
        }

        return nullptr;
    }
};

class native_library_function_getter : public library_function_getter
{
  public:
    void* create_interface() const
    {
        return find("CreateInterface");
    }
};
} // namespace fd::detail