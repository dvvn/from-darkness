module;

#include <windows.h>
#include <winternl.h>

export module fd.rt_modules:find_library;
export import fd.string;

LDR_DATA_TABLE_ENTRY* find_library(const fd::wstring_view name, const bool notify = true);
// it calls only once
LDR_DATA_TABLE_ENTRY* find_current_library(const bool notify = true);

export namespace fd
{
    using ::find_current_library;
    using ::find_library;
} // namespace fd
