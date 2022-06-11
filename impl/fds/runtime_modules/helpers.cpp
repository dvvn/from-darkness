module;

#include <fds/core/assert.h>

#include <windows.h>
#include <winternl.h>

module fds.rt_modules:helpers;

dos_nt::dos_nt(LDR_DATA_TABLE_ENTRY* const ldr_entry)
{
    dos = ldr_entry->DllBase;
    // check for invalid DOS / DOS signature.
    FDS_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    nt = dos + dos->e_lfanew;
    // check for invalid NT / NT signature.
    FDS_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
}
