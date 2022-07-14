module;

#include <string_view>

#include <windows.h>
#include <winternl.h>

module fd.rt_modules;
import :library_info;

std::wstring_view current_module::_Name() const
{
    return fd::library_info(fd::find_current_library()).name();
}

interface_finder<current_module> current_module::_Ifc_finder(const fd::basic_address<void> addr) const
{
    return addr;
}

LDR_DATA_TABLE_ENTRY* current_module::operator->() const
{
    return fd::find_current_library();
}
