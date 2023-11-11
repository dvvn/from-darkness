#include "library_info/system.h"
#include "algorithm/find.h"
#include "iterator/unwrap.h"
#include "memory/pattern.h"
#include "memory/xref.h"
#include "string/view.h"

#include <fmt/format.h>

#include <Windows.h>

#include <cassert>

namespace fd
{
void* system_library_info::function(char const* name, size_t const length) const
{
    union
    {
        IMAGE_DOS_HEADER* dos_header;
        uint8_t* base_address;
    };

    base_address = safe_cast<uint8_t>(this->base());

    union
    {
        IMAGE_EXPORT_DIRECTORY* export_dir;
        uint8_t* virtual_addr_start;
    };

    auto const entry_export = this->directory(IMAGE_DIRECTORY_ENTRY_EXPORT);
    virtual_addr_start      = base_address + entry_export->VirtualAddress;
    // auto virtual_addr_end = virtual_addr_start + entry_export->Size;

    auto const names = reinterpret_cast<uint32_t*>(base_address + export_dir->AddressOfNames);
    auto const funcs = reinterpret_cast<uint32_t*>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = reinterpret_cast<uint16_t*>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name = base_address + names[offset];
        if (fn_name[length] != '\0')
            continue;
        if (memcmp(fn_name, name, length) != 0)
            continue;

        void* fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return fn;
    }

    return nullptr;
}

static void* find_rtti_descriptor(string_view const name, uint8_t* image_base, uint8_t* image_end)
{
    uint8_t* ret;
    if (auto const space = name.find(' '); space == name.npos)
    {
        ret = find(image_base, image_end, make_pattern(".?A", 1, "name, \"@@\""));
    }
    else
    {
        auto const object_tag = [object_name = name.substr(0, space)] {
            if (object_name == "struct")
                return 'U';
            if (object_name == "class")
                return 'V';
            // todo: union
            unreachable();
        }();
        auto const class_name = name.substr(space + 1);

        array<char, 64> buff;
        ret = find(image_base, image_end, buff.begin(), fmt::format_to(buff.begin(), ".?A{}{}@@", object_tag, class_name));
    }
    return ret;
}

void* system_library_info::vtable(char const* name, size_t length) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void* rtti_descriptor;
        char const* rtti_descriptor_view;
    };

    auto const [image_start, image_end] = detail::unwrap_range(*this);

    rtti_descriptor = find_rtti_descriptor({name, length}, image_start, image_end);

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor(rtti_descriptor_address - sizeof(uintptr_t) * 2);

    // dos + section->VirtualAddress, section->SizeOfRawData

    library_section_view const rdata(this->section(".rdata"), image_start);
    auto const [rdata_begin, rdata_end] = detail::unwrap_range(rdata);

    auto const addr1 = find(rdata_begin, rdata_end, type_descriptor, [](uint8_t const* found) -> bool {
        return *unsafe_cast<uint32_t*>(found - 0x8) == 0;
    });
    if (addr1 == rdata_end)
        return nullptr;

    auto const addr2 = find(rdata_begin, rdata_end, xref(unsafe_cast<uintptr_t>(addr1) - 0xC));
    if (addr2 == rdata_end)
        return nullptr;

    library_section_view const text(this->section(".text"), image_start);
    auto const test_end = text.end();
    auto const found    = find(text.begin(), test_end, xref(unsafe_cast<uintptr_t>(addr2) + 4));
    if (found == test_end)
        return nullptr;

    return detail::unwrap_iterator(found);
}

system_library_info literals::operator""_dll(wchar_t const* name, size_t length)
{
    return {name, length, L".dll", 4};
}
} // namespace fd
