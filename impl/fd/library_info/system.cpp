#include "system.h"

#include "fd/mem_scanner.h"
#include "fd/tool/string.h"
#include "fd/tool/vector.h"

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

    auto entry_export     = this->directory(IMAGE_DIRECTORY_ENTRY_EXPORT);
    virtual_addr_start    = base_address + entry_export->VirtualAddress;
    auto virtual_addr_end = virtual_addr_start + entry_export->Size;

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
        if (string_view::traits_type::compare(fn_name, name.data(), name.length()) != 0)
            continue;

        void *fn = base_address + funcs[ords[offset]];
        assert(fn > virtual_addr_start && fn < virtual_addr_end); // fwd export not implemented
        return fn;
    }

    return nullptr;
}

void *system_library_info::pattern(string_view pattern) const
{
    auto base = static_cast<uint8_t *>(this->image_base());
    return find_pattern(base, base + this->length(), pattern.data(), pattern.length());
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

    auto find_rtti = [=,&rtti_descriptor=rtti_descriptor]<typename P>(P symbol) {
        static_vector<P, 64> full_descriptor;

        auto writer = [&]<typename A>(A const &arg) {
            if constexpr (std::convertible_to<A, P>)
                full_descriptor.push_back(arg);
            else
                std::copy(
                    std::begin(arg),
                    std::end(arg) - std::is_bounded_array_v<A>, //
                    std::back_inserter(full_descriptor));
        };

        writer(".?A");
        writer(symbol);
        writer(class_name);
        writer("@@");

        if constexpr (std::same_as<P, special_pattern_tag>)
            rtti_descriptor = find_pattern(image_base, image_end, full_descriptor.data(), full_descriptor.size());
        else
            rtti_descriptor = find_bytes(image_base, image_end, full_descriptor.data(), full_descriptor.size());
    };

    if (auto space = name.find(' '); space == name.npos)
    {
        class_name = name;
        find_rtti(special_pattern_gap);
    }
    else
    {
        auto info  = name.substr(0, space - 1);
        class_name = name.substr(space);

        if (info == "struct")
            find_rtti('U');
        else if (info == "class")
            find_rtti('V');
        else
            std::unreachable();
    }

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    auto type_descriptor = rtti_descriptor_address - sizeof(uintptr_t) * 2;

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto rdata       = this->section(".rdata");
    auto rdata_begin = image_base + rdata->VirtualAddress;
    auto rdata_end   = rdata_begin + rdata->SizeOfRawData;

    auto text       = this->section(".text");
    auto text_begin = image_base + text->VirtualAddress;
    auto text_end   = rdata_begin + text->SizeOfRawData;

    auto addr1          = find_xref(rdata_begin, rdata_end, type_descriptor, [&](uintptr_t &xref, auto *stop_token) {
        // get offset of vtable in complete class, 0 means it's the class we need, and not
        // some class it inherits from
        auto offset = *reinterpret_cast<uint32_t *>(xref - 0x8);
        if (offset == 0)
            stop_token->stop();
        return true;
    });
    auto object_locator = addr1 - 0xC;
    auto addr2          = find_xref(rdata_begin, rdata_end, object_locator) + 0x4;
    // check is valid offset
    assert(addr2 != 0x4);
    auto found = reinterpret_cast<void *>(find_xref(text_begin, text_end, addr2));
    return found;
}
} // namespace fd