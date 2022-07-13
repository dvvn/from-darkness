module;

#include <windows.h>
#include <winternl.h>

export module fd.rt_modules:helpers;
export import fd.address;

struct dos_nt
{
    dos_nt(LDR_DATA_TABLE_ENTRY* const ldr_entry);

    // base address
    fd::basic_address<IMAGE_DOS_HEADER> dos;
    fd::basic_address<IMAGE_NT_HEADERS> nt;
};

export namespace fd
{
    using ::dos_nt;
} // namespace fd
