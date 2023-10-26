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
class object_tag;
} // namespace fd

FMT_BEGIN_NAMESPACE
FMT_FORMAT_AS(fd::object_tag, char);
FMT_END_NAMESPACE

namespace fd
{
class object_tag
{
    string_view obj_;

  public:
    object_tag(string_view const obj)
        : obj_(obj)
    {
    }

    operator string_view() const
    {
        return obj_;
    }

    operator char() const
    {
        if (obj_ == "struct")
            return 'U';
        if (obj_ == "class")
            return 'V';
        // todo: union
        unreachable();
    }
};

void* system_library_info::function(char const* name, size_t const length) const
{
    union
    {
        IMAGE_DOS_HEADER* dos_header;
        uint8_t* base_address;
    };

    base_address = static_cast<uint8_t*>(this->base());

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
        auto const fn_name = (base_address + names[offset]);
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
    void* ret;
    if (auto const space = name.find(' '); space == name.npos)
    {
        auto const pat = make_pattern(".?A", 1, "name, \"@@\""); // todo: FIX ME!!!
        ret            = find(image_base, image_end, pat);
    }
    else
    {
        object_tag const info(name.substr(0, space));
        auto const class_name = name.substr(space + 1);

        array<char, 64> buff;
        auto const buff_end = fmt::format_to(buff.begin(), ".?A{}{}@@", info, class_name);

        ret = find(image_base, image_end, (buff.begin()), (buff_end));
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

    auto const image_base = static_cast<uint8_t*>(this->image_base());
    auto const image_end  = (image_base) + this->length();

    rtti_descriptor = find_rtti_descriptor({name, length}, image_base, image_end);

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor(rtti_descriptor_address - sizeof(uintptr_t) * 2);

    // dos + section->VirtualAddress, section->SizeOfRawData

    library_section_view const rdata(this->section(".rdata"), image_base);
    auto const rdata_begin = unwrap_iterator(rdata.begin());
    auto const rdata_end   = unwrap_iterator(rdata.end());

    auto const addr1 = find(rdata_begin, rdata_end, type_descriptor, [](uint8_t const* found) -> bool {
        return *unsafe_cast<uint32_t*>(found - 0x8) == 0;
    });
    if (!addr1)
        return nullptr;
    auto const addr2 = find(rdata_begin, rdata_end, xref(safe_cast<uint8_t*>(addr1) - 0xC));
    if (!addr2)
        return nullptr;

    library_section_view const text(this->section(".text"), image_base);
    auto const test_end = text.end();
    auto const found    = fd::find(text.begin(), text.end(), xref(unsafe_cast<uintptr_t>((addr2)) + 4));
    return found != test_end ? unwrap_iterator(found) : nullptr;
}
} // namespace fd
