#include "library_name.h"

#include <fd/tool/string_view.h>

#include <windows.h>
#include <winternl.h>

#include <cassert>

namespace fd
{
system_string_view library_name(system_string_view path);

system_string_view library_path(system_library_entry entry)
{
    system_string_view path;
    auto &ustr = entry->FullDllName;
    if (ustr.Buffer)
        path = {ustr.Buffer, ustr.Length / sizeof(WCHAR)};
    return path;
}

system_string_view library_name(system_library_entry entry)
{
    return library_name(library_path(entry));
}

system_string_view library_name(system_string_view path)
{
    auto name_start = path.rfind('\\');
    assert(name_start != path.npos);
    return path.substr(name_start + 1);
}

system_string_view library_name(system_library_entry entry, size_t limit)
{
    using l = std::numeric_limits<size_t>;
    assert(limit != l::min() && limit != l::max());
    auto path = library_path(entry);
    assert(path.length() > limit);
    path.remove_prefix(path.length() - (limit + 1));
    auto name_start = path.rfind('\\');
    system_string_view ret;
    if (name_start != path.npos)
    {
        ret = path.substr(name_start + 1);
        assert(!ret.empty());
    }
    return ret;
}

bool library_has_name(system_library_entry entry, system_string_view name)
{
    auto path = library_path(entry);
    path.remove_prefix(path.length() - (name.length() + 1));
    return path.starts_with('\\') && path.ends_with(name);
}

bool valid_library_name(system_string_view path, system_string_view name)
{
    return path.ends_with(name);
}
}