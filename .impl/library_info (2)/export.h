#pragma once

#include "core.h"

namespace fd
{
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, char const *name, size_t length);
void *find_export(IMAGE_DOS_HEADER *dos, IMAGE_NT_HEADERS *nt, struct string_view name);
}