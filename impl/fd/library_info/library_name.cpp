#include "library_name.h"

#include <windows.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
system_string_view library_name(system_string_view path);
bool valid_library_name(system_string_view path, system_string_view name);

system_string_view library_path(LDR_DATA_TABLE_ENTRY *entry)
{
    system_string_view path;
    auto &ustr = entry->FullDllName;
    if (ustr.Buffer)
        path = {ustr.Buffer, ustr.Length / sizeof(WCHAR)};
    return path;
}

system_string_view library_name(LDR_DATA_TABLE_ENTRY *entry)
{
    return library_name(library_path(entry));
}

bool valid_library_name(LDR_DATA_TABLE_ENTRY *entry, system_string_view name)
{
    auto path = library_path(entry);
    return valid_library_name(path, name) && *next(path.rbegin(), name.size()) == '\\';
}

system_string_view library_name(system_string_view path)
{
    auto name_start = path.rfind('\\');
    assert(name_start != path.npos);
    return path.substr(name_start + 1);
}

bool valid_library_name(system_string_view path, system_string_view name)
{
    return path.ends_with(name);
}
}