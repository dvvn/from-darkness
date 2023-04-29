#include <fd/library_info/dos.h>
#include <fd/library_info/vtable.h>
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
static void *find_type_descriptor(IMAGE_NT_HEADERS *nt, char const *name, size_t length)
{
    auto scanner = memory_scanner(nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfImage);
    for (auto begin : scanner(".?A", 3))
    {
        auto c_begin = static_cast<char const *>(begin);
        if constexpr (Prefix != 0)
        {
            if (c_begin[3] != Prefix)
                continue;
        }
        if (validate_rtti_name(c_begin, name, length))
            return begin;
    }
    return nullptr;
}

struct xrefs_scanner_dos : xrefs_scanner
{
    xrefs_scanner_dos(dos_header dos, IMAGE_SECTION_HEADER *section)
        : xrefs_scanner(dos + section->VirtualAddress, section->SizeOfRawData)
    {
    }
};

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
    auto find = [=](size_t offset, auto finder) {
        return finder(nt, name + offset, length - offset);
    };

    if (auto offset = find_offset("struct "))
        return find(offset, find_type_descriptor<'U'>);
    if (auto offset = find_offset("class "))
        return find(offset, find_type_descriptor<'V'>);

    return find_type_descriptor(nt, name, length);
}

template <size_t S>
static bool validate_section(IMAGE_SECTION_HEADER *s, char const (&name)[S])
{
    return memcmp(s->Name, name, S - 1) == 0;
}

void *find_vtable(IMAGE_SECTION_HEADER *rdata, IMAGE_SECTION_HEADER *text, IMAGE_DOS_HEADER *dos, void *rtti_decriptor)
{
    assert(validate_section(rdata, ".rdata"));
    assert(validate_section(text, ".text"));

    // get rtti type descriptor
    auto type_descriptor = reinterpret_cast<uintptr_t>(rtti_decriptor);
    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    type_descriptor -= sizeof(uintptr_t) * 2;

    auto rdata_scanner = xrefs_scanner_dos(dos, rdata);
    auto text_scanner  = xrefs_scanner_dos(dos, text);

    for (auto xref : rdata_scanner(type_descriptor))
    {
        // get offset of vtable in complete class, 0 means it's the class we need, and not some class it inherits from
        auto offset = *reinterpret_cast<uint32_t *>(xref - 0x8);
        if (offset != 0)
            continue;

        auto object_locator = xref - 0xC;
        auto addr           = rdata_scanner(object_locator).front() + 0x4;

        // check is valid offset
        assert(addr > sizeof(uintptr_t));
        return reinterpret_cast<void *>(text_scanner(addr).front());
    }
    return nullptr;
}
}