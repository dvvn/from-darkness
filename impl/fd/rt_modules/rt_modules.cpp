

module;

#include <fd/assert.h>

#include <windows.h>
#include <winternl.h>

module fd.rt_modules;

only_true::only_true(const bool val)
{
    FD_ASSERT(val == true);
}

using fd::library_info;

const library_info* any_module_base::operator->() const
{
    return &data();
}

const library_info& any_module_base::operator*() const
{
    return data();
}

static DECLSPEC_NOINLINE HMODULE _Get_current_module_handle()
{
    MEMORY_BASIC_INFORMATION info;
    constexpr size_t info_size      = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _Get_current_module_handle, &info, info_size);
    FD_ASSERT(len == info_size, "Wrong size");
    return static_cast<HMODULE>(info.AllocationBase);
}

const library_info& current_module::data() const
{
    static const library_info current((IMAGE_DOS_HEADER*)_Get_current_module_handle());
    return current;
}

bool current_module::loaded() const
{
    return true;
}
