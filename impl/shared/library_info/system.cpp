#include "pattern.h"
#include "system.h"
#include "xref.h"
#include "algorithm/find.h"
#include "container/vector/static.h"
#include "string/view.h"

#include <Windows.h>

#include <cassert>

namespace fd
{
void *system_library_info::function(string_view name) const
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

    auto entry_export  = this->directory(IMAGE_DIRECTORY_ENTRY_EXPORT);
    virtual_addr_start = base_address + entry_export->VirtualAddress;
    // auto virtual_addr_end = virtual_addr_start + entry_export->Size;

    auto names = reinterpret_cast<uint32_t *>(base_address + export_dir->AddressOfNames);
    auto funcs = reinterpret_cast<uint32_t *>(base_address + export_dir->AddressOfFunctions);
    auto ords  = reinterpret_cast<uint16_t *>(base_address + export_dir->AddressOfNameOrdinals);

    //----

    auto last_offset = std::min(export_dir->NumberOfNames, export_dir->NumberOfFunctions);
    for (DWORD offset = 0; offset != last_offset; ++offset)
    {
        auto fn_name = reinterpret_cast<char const *>(base_address + names[offset]);
        if (fn_name[name.length()] != '\0')
            continue;
        if (memcmp(fn_name, name.data(), name.length()) != 0)
            continue;

        void *fn = base_address + funcs[ords[offset]];
        // assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return fn;
    }

    return nullptr;
}

void *system_library_info::pattern(basic_pattern const &pattern) const
{
    auto base = static_cast<uint8_t *>(this->image_base());
    return find(base, base + this->length(), pattern);
}

void *system_library_info::vtable(string_view name) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void *rtti_descriptor;
        char const *rtti_descriptor_view;
    };

    string_view class_name;

    auto image_base = static_cast<uint8_t *>(this->image_base());
    auto image_end  = image_base + this->length();

    constexpr string_view rtti_begin = ".?A";
    constexpr string_view rtti_end   = "@@";

    if (auto space = name.find(' '); space == name.npos)
    {
        class_name      = name;
        rtti_descriptor = find(image_base, image_end, make_pattern(rtti_begin, 1, class_name, rtti_end));
    }
    else
    {
        auto info  = name.substr(0, space - 1);
        class_name = name.substr(space);

        static_vector<char, 64> buff;
        auto raw_length = rtti_begin.length() + 1 + class_name.length() + rtti_end.length();
        buff.resize(raw_length);
        auto it = std::copy(rtti_begin.begin(), rtti_begin.end(), buff.begin());
        if (info == "struct")
            *it++ = 'U';
        else if (info == "class")
            *it++ = 'V';
        else
            std::unreachable();
        it = std::copy(class_name.begin(), class_name.end(), it);
        std::copy(rtti_end.begin(), rtti_end.end(), it);

        rtti_descriptor = find(image_base, image_end, buff.data(), buff.data() + buff.size());
    }

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref type_descriptor = rtti_descriptor_address - sizeof(uintptr_t) * 2;

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto rdata       = this->section(".rdata");
    auto rdata_begin = image_base + rdata->VirtualAddress;
    auto rdata_end   = rdata_begin + rdata->SizeOfRawData;

    auto text       = this->section(".text");
    auto text_begin = image_base + text->VirtualAddress;
    auto text_end   = text_begin + text->SizeOfRawData;

    void *addr1;
    for (auto begin = rdata_begin;;)
    {
        auto tmp = static_cast<uint8_t *>(find(begin, rdata_end, type_descriptor));
        if (!tmp)
            return nullptr;
        auto offset = *reinterpret_cast<uint32_t *>(tmp - 0x8);
        if (offset == 0)
        {
            addr1 = (tmp);
            break;
        }
        begin = tmp + sizeof(uintptr_t);
    }

    xref object_locator = static_cast<uint8_t *>(addr1) - 0xC;
    auto addr2          = find(rdata_begin, rdata_end, object_locator);

    if (!addr2)
        return nullptr;

    return find(text_begin, text_end, xref(reinterpret_cast<uintptr_t>(addr2) + 4));
}
} // namespace fd