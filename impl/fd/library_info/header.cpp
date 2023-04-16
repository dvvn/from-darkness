#include <fd/library_info/header.h>

#include <cassert>
#include <cstdint>

namespace fd
{
IMAGE_DOS_HEADER *_get_dos(LDR_DATA_TABLE_ENTRY *entry)
{
    auto dos = static_cast<IMAGE_DOS_HEADER *>(entry->DllBase);
    // check for invalid DOS / DOS signature.
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    return dos;
}

IMAGE_NT_HEADERS *_get_nt(IMAGE_DOS_HEADER *dos)
{
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<uint8_t *>(dos) + dos->e_lfanew);
    // check for invalid NT / NT signature.
    assert(nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
    return nt;
}
}