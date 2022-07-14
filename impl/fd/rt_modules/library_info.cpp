module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

module fd.rt_modules:library_info;
import fd.logger;

library_info::library_info(pointer const entry)
    : entry_(entry)
{
    FD_ASSERT(entry != nullptr);
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
    return { entry_->FullDllName.Buffer, entry_->FullDllName.Length / sizeof(WCHAR) };
}

std::wstring_view library_info::name() const
{
    const auto full_path  = this->path();
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
}

void library_info::log(const std::string_view object_type, const std::string_view object, const void* addr) const
{
    std::invoke(
        fd::logger, L"{} -> {} '{}' {}! ({:#X})",
        [=] {
            return this->name();
        },
        object_type, object,
        [=] {
            return addr ? "found" : "not found";
        },
        reinterpret_cast<uintptr_t>(addr));
}
