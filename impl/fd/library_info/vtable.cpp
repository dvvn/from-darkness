#include "vtable.h"

#include <fd/magic_cast.h>
#include <fd/mem_scanner.h>

#include <windows.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
static bool validate_rtti_name(char const *begin, char const *name, size_t length)
{
    auto name_begin = begin + 3 + 1; //.?A_
    if (memcmp(name_begin + length, "@@", 2) != 0)
        return false;
    if (memcmp(name_begin, name, length) != 0)
        return false;
    return true;
}

template <char Prefix = 0>
static void *find_type_descriptor(to<uint8_t *> begin, size_t length, char const *name, size_t name_length)
{
    void *found = nullptr;

    // to<uint8_t *> begin = nt->OptionalHeader.ImageBase;
    // auto end            = begin + nt->OptionalHeader.SizeOfImage;

    find_bytes(begin, begin + length, to<void *>(".?A"), 3, [&](to<char *> tmp) -> bool {
        if constexpr (Prefix != 0)
        {
            if (tmp[3] != Prefix)
                return true;
        }
        if (!validate_rtti_name(tmp, name, name_length))
            return true;
        found = tmp;
        return false;
    });

    return found;
}

template <char Prefix = 0>
static void *find_type_descriptor(IMAGE_NT_HEADERS *nt, char const *name, size_t name_length)
{
    return find_type_descriptor<Prefix>(nt, nt->OptionalHeader.SizeOfImage, name, name_length);
}

void *find_rtti_descriptor(IMAGE_NT_HEADERS *nt, char const *name, size_t length)
{
    auto find_offset = [=]<size_t S>(char const(&sample)[S]) -> size_t {
        constexpr auto sample_length = S - 1;
        if (length <= sample_length)
            return 0;
        if (memcmp(name, sample, sample_length) != 0)
            return 0;
        return sample_length;
    };

    if (auto offset = find_offset("struct "))
        return find_type_descriptor<'U'>(nt, name + offset, length - offset);
    if (auto offset = find_offset("class "))
        return find_type_descriptor<'V'>(nt, name + offset, length - offset);

    return find_type_descriptor(nt, name, length);
}

template <size_t S>
static bool validate_section(IMAGE_SECTION_HEADER *s, char const (&name)[S])
{
    return memcmp(s->Name, name, S - 1) == 0;
}

static to<void *> find_vtable_impl(
    IMAGE_SECTION_HEADER *rdata,
    IMAGE_SECTION_HEADER *text,
    /*IMAGE_DOS_HEADER **/ to<uint8_t *> dos,
    void *rtti_decriptor)
{
    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_decriptor);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    type_descriptor -= sizeof(uintptr_t) * 2;

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto rdata_begin = (dos) + rdata->VirtualAddress;
    auto rdata_end   = rdata_begin + rdata->SizeOfRawData;

    auto text_begin = dos + text->VirtualAddress;
    auto text_end   = rdata_begin + text->SizeOfRawData;

    uintptr_t found = 0;

    find_xref(rdata_begin, rdata_end, type_descriptor, [&](uintptr_t &xref) {
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it
        // inherits from
        auto offset = *reinterpret_cast<uint32_t *>(xref - 0x8);
        if (offset != 0)
            return true;

        auto object_locator = xref - 0xC;
        auto addr           = find_xref(rdata_begin, rdata_end, object_locator) + 0x4;

        // check is valid offset
        assert(addr > sizeof(uintptr_t));
        found = find_xref(text_begin, text_end, addr);
        return found != 0;
    });
    return found;
}

void *find_vtable(IMAGE_SECTION_HEADER *rdata, IMAGE_SECTION_HEADER *text, IMAGE_DOS_HEADER *dos, void *rtti_decriptor)
{
    assert(validate_section(rdata, ".rdata"));
    assert(validate_section(text, ".text"));

    return find_vtable_impl(rdata, text, dos, rtti_decriptor);
}
}