module;

#include <fd/assert.h>

#include "dll_notification.h"

#include <iterator>
#include <ranges>

module fd.rt_modules;
import fd.filesystem.path;
import fd.semaphore;
import fd.string.make;

using namespace fd;

struct callback_data_t
{
    wstring_view name;
    semaphore sem = { 0, 1 };
    library_info found;
};

static void CALLBACK _On_new_library(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context)
{
    const auto data = static_cast<callback_data_t*>(Context);

    if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_UNLOADED)
    {
        if (NotificationData->Unloaded.DllBase != (*rt_modules::current)->DllBase)
            return;
    }
    else
    {
        const auto& full_name = *NotificationData->Loaded.FullDllName;
        const wstring_view target_name(full_name.Buffer, full_name.Length / sizeof(WCHAR));

        if (!target_name.ends_with(data->name))
            return;
        data->found = static_cast<IMAGE_DOS_HEADER*>(NotificationData->Loaded.DllBase);
    }

    data->sem.release();
}

library_info _Wait_for_library(const wstring_view name)
{
    const auto existing = library_info::find(name);

    if (existing)
        return existing;

    static auto LdrRegisterDllNotification_fn   = (LdrRegisterDllNotification)rt_modules::ntDll->find_export("LdrRegisterDllNotification");
    static auto LdrUnregisterDllNotification_fn = (LdrUnregisterDllNotification)rt_modules::ntDll->find_export("LdrUnregisterDllNotification");

    callback_data_t cb_data = { name };
    void* cookie;
    if (LdrRegisterDllNotification_fn(0, _On_new_library, &cb_data, &cookie) != STATUS_SUCCESS)
        return {};
    if (!cb_data.sem.acquire())
        return {};
    if (LdrUnregisterDllNotification_fn(cookie) != STATUS_SUCCESS)
        return {};
    return cb_data.found;
}

const library_info* any_module_base::operator->() const
{
    return &data();
}

const library_info& any_module_base::operator*() const
{
    return data();
}

static DECLSPEC_NOINLINE PVOID _Get_current_module_handle()
{
    MEMORY_BASIC_INFORMATION info;
    constexpr size_t info_size      = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _Get_current_module_handle, &info, info_size);
    FD_ASSERT(len == info_size, "Wrong size");
    return info.AllocationBase;
}

const library_info& current_module::data() const
{
    static const library_info current(static_cast<IMAGE_DOS_HEADER*>(_Get_current_module_handle()));
    return current;
}

bool current_module::wait() const
{
    return true;
}

//----

unknown_module::unknown_module(const wstring_view name, const bool exact, const string_view extension)
{
    if (exact)
        name_ = name;
    else
        name_ = make_string(name | std::views::transform(to_lower), extension | std::views::transform(to_lower));

    info_ = library_info::find(name_);
}

unknown_module::unknown_module(unknown_module&& other)
{
    *this = std::move(other);
}

unknown_module& unknown_module::operator=(unknown_module&& other)
{
    using std::swap;
    swap(name_, other.name_);
    swap(info_, other.info_);
    return *this;
}

const library_info& unknown_module::data() const
{
    if (!info_)
    {
#ifdef _DEBUG
        const auto lib = _Wait_for_library(name_);
        FD_ASSERT(static_cast<bool>(lib));
        info_ = lib;
#else
        info_ = name_;
#endif
    }

    return info_;
}

bool unknown_module::wait() const
{
    return info_ || _Wait_for_library(name_);
}
