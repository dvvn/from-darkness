#pragma once

#include <cstdint>

namespace fd
{
int hde_disasm(void* src, int* reloc_op_offset);
}