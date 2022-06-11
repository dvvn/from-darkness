module;

#include <fds/core/assert.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

module fds.rt_modules:library_info;

library_info::library_info(pointer const entry)
    : entry_(entry)
{
}

auto library_info::operator->() const -> pointer
{
    return entry_;
}

auto library_info::operator*() const -> reference
{
    return *entry_;
}

std::wstring_view library_info::path() const
{
    return {entry_->FullDllName.Buffer, entry_->FullDllName.Length / sizeof(WCHAR)};
}

std::wstring_view library_info::name() const
{
    const auto full_path  = this->path();
    const auto name_start = full_path.rfind('\\');
    FDS_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
}
