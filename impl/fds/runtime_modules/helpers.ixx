module;

#include <windows.h>
#include <winternl.h>

export module fds.rt_modules:helpers;
export import fds.address;

struct dos_nt
{
    dos_nt(LDR_DATA_TABLE_ENTRY* const ldr_entry);

    // base address
    fds::basic_address<IMAGE_DOS_HEADER> dos;
    fds::basic_address<IMAGE_NT_HEADERS> nt;
};

export namespace fds
{
    using ::dos_nt;
} // namespace fds
