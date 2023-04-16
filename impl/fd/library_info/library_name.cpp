#include <fd/library_info/library_name.h>

#include <windows.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
std::wstring_view library_path(LDR_DATA_TABLE_ENTRY *entry)
{
    auto &ustr = entry->FullDllName;
    if (ustr.Buffer)
        return { ustr.Buffer, ustr.Length / sizeof(WCHAR) };
    return {};
}

std::wstring_view library_name(LDR_DATA_TABLE_ENTRY *entry)
{
    return library_name(library_path(entry));
}

std::wstring_view library_name(std::wstring_view path)
{
    auto name_start = path.rfind('\\');
    assert(name_start != path.npos);
    return path.substr(name_start + 1);
}
}