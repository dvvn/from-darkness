#include "library_info/system.h"
#include "algorithm/find_pattern.h"
#include "functional/cast.h"
#include "iterator/unwrap.h"
#include "memory/pattern.h"
#include "memory/xref.h"
#include "string/view.h"

#include <fmt/format.h>

#include <Windows.h>

namespace fd
{
void* system_library_info::function(string_view name) const
{
    auto const base_address = (this->base());

    data_dir_range const data_dirs(this->nt_header(base_address));
    auto const& entry_export = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto const export_dir    = safe_cast<IMAGE_EXPORT_DIRECTORY>(base_address + entry_export.VirtualAddress);
    // auto const export_dir_end = export_dir_start+entry_export.Size;

    auto const names = safe_cast<uint32_t>(base_address + export_dir->AddressOfNames);
    auto const funcs = safe_cast<uint32_t>(base_address + export_dir->AddressOfFunctions);
    auto const ords  = safe_cast<uint16_t>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto const name_ufirst = ubegin(name);
    auto const name_ulast  = uend(name);
    auto const name_length = name.length();

    auto const last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto const fn_name_raw = safe_cast<char>(base_address + names[offset]);
        if (fn_name_raw[name_length] != '\0')
            continue;
        if (!std::equal(fn_name_raw, fn_name_raw + name_length, name_ufirst, name_ulast))
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
        ret = find(image_base, image_end, make_pattern(".?A", 1, name, 0, "\"@@\""));
    }
    else
    {
        auto const object_name = name.substr(0, space);
        char object_tag;

        if (object_name == "struct")
            object_tag = 'U';
        else if (object_name == "class")
            object_tag = 'V';
        else
            unreachable();

        auto const class_name = name.substr(space + 1);

        array<char, 64> buff;
        ret = find(image_base, image_end, buff.begin(), fmt::format_to(buff.begin(), ".?A{}{}@@", object_tag, class_name));
    }
    return ret;
}

void* system_library_info::vtable(string_view name) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void* rtti_descriptor;
        char const* rtti_descriptor_view;
    };

    auto const image_start = ubegin(*this);
    auto const image_end   = uend(*this);

    auto const nt = nt_header();
    sections_range const sections(nt);

    rtti_descriptor = find_rtti_descriptor(name, image_start, image_end);

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor(rtti_descriptor_address - sizeof(uintptr_t) * 2);

    // dos + section->VirtualAddress, section->SizeOfRawData

    section_view const rdata(find(sections, ".rdata"), image_start);

    auto const rdata_begin = ubegin(rdata);
    auto const rdata_end   = ubegin(rdata);

    auto const addr1 = find(rdata_begin, rdata_end, type_descriptor, [](uint8_t const* found) -> bool {
        return *unsafe_cast<uint32_t*>(found - 0x8) == 0;
    });
    if (addr1 == rdata_end)
        return nullptr;

    auto const addr2 = find(rdata_begin, rdata_end, xref(unsafe_cast<uintptr_t>(addr1) - 0xC));
    if (addr2 == rdata_end)
        return nullptr;

    section_view const text(find(sections, ".text"), image_start);
    auto const test_end = uend(text);
    auto const found    = find(ubegin(text), test_end, xref(unsafe_cast<uintptr_t>(addr2) + 4));
    if (found == test_end)
        return nullptr;

    return found;
}

system_library_info literals::operator""_dll(wchar_t const* name, size_t length)
{
    return {{name, length}, system_library_info::extension_tag::dll};
}
} // namespace fd
