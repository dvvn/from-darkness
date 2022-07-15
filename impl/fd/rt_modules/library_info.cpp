module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

#include <functional>

module fd.rt_modules:library_info;
import fd.logger;
import fd.path;

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

fd::wstring_view library_info::path() const
{
    return { entry_->FullDllName.Buffer, entry_->FullDllName.Length / sizeof(WCHAR) };
}

fd::wstring_view library_info::name() const
{
    const auto full_path = this->path();
#if 1
    return fd::path(full_path).filename();
#else
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
#endif
}

void library_info::log(const fd::string_view object_type, const fd::string_view object, const void* addr) const
{
    std::invoke(
        fd::logger, L"{} -> {} '{}' {}! ({:#X})", std::bind_front(&library_info::name, this), object_type, object, addr ? "found" : "not found", reinterpret_cast<uintptr_t>(addr));
}
