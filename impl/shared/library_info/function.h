#pragma once

#include "library_info/holder.h"

#include <algorithm>

namespace fd::detail
{
inline void* library_function_getter<>::find(string_view const name) const
{
    auto const base_address = linfo_->data();

    auto const& data_dirs    = (linfo_->nt_header()->OptionalHeader.DataDirectory);
    auto const& entry_export = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto const export_dir    = unsafe_cast<IMAGE_EXPORT_DIRECTORY*>(base_address + entry_export.VirtualAddress);
    // auto const export_dir_end = export_dir_start+entry_export.Size;

    auto const names = unsafe_cast<uint32_t*>(base_address + export_dir->AddressOfNames);
    auto const funcs = unsafe_cast<uint32_t*>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = unsafe_cast<uint16_t*>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const name_first  = name.data();
    auto const name_length = name.length();
    auto const name_last   = name_first + name_length;

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name_raw = unsafe_cast<char const*>(base_address + names[offset]);
        if (fn_name_raw[name_length] != '\0')
            continue;
        if (!std::equal(name_first, name_last, fn_name_raw))
            continue;

        auto const fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return unsafe_cast_from(fn);
    }

    return nullptr;
}
} // namespace fd::detail