#pragma once

#include "library_info.h"

#include <algorithm>

namespace fd::detail
{
class library_function_getter
{
    uint8_t* base_address_;
    uint32_t* names_;
    uint32_t* funcs_;
    uint16_t* ords_;
    DWORD last_offset_;

  public:
    library_function_getter(library_info const* linfo)
    {
        auto const nt         = linfo->nt_header();
        base_address_         = linfo->data();
        auto const export_dir = unsafe_cast<IMAGE_EXPORT_DIRECTORY*>(
            base_address_ + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        // auto const export_dir_end = export_dir_start+entry_export.Size;
        names_ = unsafe_cast_from(base_address_ + export_dir->AddressOfNames);
        funcs_ = unsafe_cast_from(base_address_ + export_dir->AddressOfFunctions);
        ords_  = unsafe_cast_from(base_address_ + export_dir->AddressOfNameOrdinals);

        last_offset_ = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    }

    void* find(std::string_view const name) const noexcept
    {
        auto const name_first  = name.data();
        auto const name_length = name.length();
        auto const name_last   = name_first + name_length;

        for (DWORD offset = 0; offset != last_offset_; ++offset)
        {
            using std::equal;

            auto const fn_name_raw = unsafe_cast<char const*>(base_address_ + names_[offset]);
            if (fn_name_raw[name_length] != '\0')
                continue;
            if (!equal(name_first, name_last, fn_name_raw))
                continue;

            auto const fn = base_address_ + funcs_[ords_[offset]];
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