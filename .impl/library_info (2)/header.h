#pragma once

#include "core.h"

#include <cstdint>

namespace fd
{
template <typename T>
struct span;

IMAGE_DOS_HEADER *get_dos(LDR_DATA_TABLE_ENTRY *entry);
IMAGE_NT_HEADERS *get_nt(IMAGE_DOS_HEADER *dos);
span<uint8_t> get_memory_range(IMAGE_NT_HEADERS *nt);
}