module;

#include <windows.h>
#include <winternl.h>

module fd.rt_modules;
import :library_info;

using fd::current_module;

fd::wstring_view current_module::_Name() const
{
    return fd::library_info(data()).name();
}

LDR_DATA_TABLE_ENTRY* current_module::operator->() const
{
    return data();
}

LDR_DATA_TABLE_ENTRY* current_module::data() const
{
    return fd::find_current_library();
}
