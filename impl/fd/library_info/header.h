#pragma once

#include <windows.h>
#include <winternl.h>

namespace fd
{
IMAGE_DOS_HEADER *get_dos(LDR_DATA_TABLE_ENTRY *entry);
IMAGE_NT_HEADERS *get_nt(IMAGE_DOS_HEADER *dos);
// IMAGE_NT_HEADERS *_get_nt(LDR_DATA_TABLE_ENTRY *entry);
}