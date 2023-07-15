#include "pattern.h"
#include "system.h"
#include "xref.h"
#include "algorithm/find.h"
#include "iterator/unwrap.h"
#include "string/view.h"

#include <fmt/format.h>

#include <Windows.h>

#include <cassert>

namespace fd
{
void *system_library_info::function(char const *name, size_t length) const
{
    union
    {
        IMAGE_DOS_HEADER *dos_header;
        uint8_t *base_address;
    };

    base_address = static_cast<uint8_t *>(this->base());

    union
    {
        IMAGE_EXPORT_DIRECTORY *export_dir;
        uint8_t *virtual_addr_start;
    };

    auto const entry_export = this->directory(IMAGE_DIRECTORY_ENTRY_EXPORT);
    virtual_addr_start      = base_address + entry_export->VirtualAddress;
    // auto virtual_addr_end = virtual_addr_start + entry_export->Size;

    auto const names = reinterpret_cast<uint32_t *>(base_address + export_dir->AddressOfNames);
    auto const funcs = reinterpret_cast<uint32_t *>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = reinterpret_cast<uint16_t *>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name = (base_address + names[offset]);
        if (fn_name[length] != '\0')
            continue;
        if (memcmp(fn_name, name, length) != 0)
            continue;

        void *fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return fn;
    }

    return nullptr;
}

void *system_library_info::pattern(basic_pattern const &pattern) const
{
    auto const base = static_cast<uint8_t *>(this->image_base());
    return find(base, base + this->length(), pattern);
}

static void *find_rtti_descriptor(string_view name, void *image_base, void *image_end)
{
    if (auto const space = name.find(' '); space == name.npos)
    {
        auto pat = make_pattern(".?A", 1, name, "@@");
        return find(image_base, image_end, pat);
    }
    else
    {
        auto const info = name.substr(0, space - 1);
        auto class_name = name.substr(space);

        array<char, 64> buff;
        auto const buff_end = fmt::format_to(
            buff.begin(), //
            ".?A{}{}@@",
            info == "struct"  ? 'U'
            : info == "class" ? 'V'
                              : (unreachable(), '\0'),
            class_name);

        return find(image_base, image_end, data(buff), iterator_to_raw_pointer(buff_end));
    }
}

void *system_library_info::vtable(char const *name, size_t length) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void *rtti_descriptor;
        char const *rtti_descriptor_view;
    };

    auto const image_base = static_cast<uint8_t *>(this->image_base());
    auto const image_end  = (image_base) + this->length();

    rtti_descriptor = find_rtti_descriptor({name, length}, image_base, image_end);

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor = rtti_descriptor_address - sizeof(uintptr_t) * 2;

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto const rdata       = this->section(".rdata");
    auto const rdata_begin = image_base + rdata->VirtualAddress;
    auto const rdata_end   = rdata_begin + rdata->SizeOfRawData;

    auto const text       = this->section(".text");
    auto const text_begin = image_base + text->VirtualAddress;
    auto const text_end   = text_begin + text->SizeOfRawData;

    void *addr1;
    for (auto begin = rdata_begin;;)
    {
        auto const tmp = static_cast<uint8_t *>(find(begin, rdata_end, type_descriptor));
        if (!tmp)
            return nullptr;
        auto const offset = *reinterpret_cast<uint32_t *>(tmp - 0x8);
        if (offset == 0)
        {
            addr1 = (tmp);
            break;
        }
        begin = tmp + sizeof(uintptr_t);
    }

    xref const object_locator = static_cast<uint8_t *>(addr1) - 0xC;
    auto addr2                = find(rdata_begin, rdata_end, object_locator);

    if (!addr2)
        return nullptr;

    return find(text_begin, text_end, xref(reinterpret_cast<uintptr_t>(addr2) + 4));
}
} // namespace fd