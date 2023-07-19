#include "pattern.h"
#include "search_stop_token.h"
#include "system.h"
#include "xref.h"
#include "algorithm/find.h"
#include "algorithm/search.h"
#include "iterator/unwrap.h"
#include "string/view.h"

#include <fmt/format.h>

#include <Windows.h>

#include <cassert>

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
}; // namespace fd

FMT_BEGIN_NAMESPACE
FMT_FORMAT_AS(fd::object_tag, char);
FMT_END_NAMESPACE

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

static void *find_rtti_descriptor(string_view const name, void *image_base, void const *image_end)
{
    if (auto const space = name.find(' '); space == name.npos)
    {
        auto const pat = make_pattern(".?A", 1, name, "@@");
        return find(image_base, image_end, pat);
    }
    else
    {
        object_tag const info(name.substr(0, space - 1));
        auto const class_name = name.substr(space);

        array<char, 64> buff;
        auto const buff_end = fmt::format_to(buff.begin(), ".?A{}{}@@", info, class_name);

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

    auto const addr1 = search(
        rdata_begin, rdata_end, type_descriptor, //
        search_stop_token([](uint8_t *found) -> bool {
            return *unsafe_cast<uint32_t *>(found - 0x8) == 0;
        }));
    if (!addr1)
        return nullptr;

    auto const addr2 = find(rdata_begin, rdata_end, xref(safe_cast<uint8_t *>(addr1) - 0xC));
    if (!addr2)
        return nullptr;

    return find(text_begin, text_end, xref(unsafe_cast<uintptr_t>(addr2) + 4));
}
} // namespace fd