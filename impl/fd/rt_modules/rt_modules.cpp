module;

#include <fd/assert.h>

#include "dll_notification.h"

module fd.rt_modules;
import fd.filesystem.path;

using namespace fd;

static DECLSPEC_NOINLINE HMODULE _Get_current_module_handle()
{
    MEMORY_BASIC_INFORMATION info;
    constexpr size_t info_size      = sizeof(MEMORY_BASIC_INFORMATION);
    // todo: is this is dll, try to load this function from inside
    [[maybe_unused]] const auto len = VirtualQueryEx(GetCurrentProcess(), _Get_current_module_handle, &info, info_size);
    FD_ASSERT(len == info_size, "Wrong size");
    return static_cast<HMODULE>(info.AllocationBase);
}

struct callback_data_t
{
    wstring_view name;
    HANDLE semaphore = CreateSemaphore(0, 0, 1, nullptr);
    bool found       = false;

    ~callback_data_t()
    {
        CloseHandle(semaphore);
    }
};

static void CALLBACK _On_new_library(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context)
{
    const auto data = static_cast<callback_data_t*>(Context);

    if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_UNLOADED)
    {
        if (NotificationData->Unloaded.DllBase != _Get_current_module_handle())
            return;
        data->found = false;
    }
    else
    {
        const auto& full_name = *NotificationData->Loaded.FullDllName;
        const wstring_view target_name(full_name.Buffer, full_name.Length / sizeof(WCHAR));

        if (!target_name.ends_with(data->name))
            return;
        data->found = true;
    }

    ReleaseSemaphore(data->semaphore, 1, 0);
}

bool _Wait_for_library(const wstring_view name)
{
    if (library_info::exists(name))
        return true;

    static library_info ntdll(L"ntdll.dll");
    static auto LdrRegisterDllNotification_fn   = (LdrRegisterDllNotification)ntdll.find_export("LdrRegisterDllNotification");
    static auto LdrUnregisterDllNotification_fn = (LdrUnregisterDllNotification)ntdll.find_export("LdrUnregisterDllNotification");

    callback_data_t cb_data = { name };
    void* cookie;
    if (LdrRegisterDllNotification_fn(0, _On_new_library, &cb_data, &cookie) != STATUS_SUCCESS)
        return false;
    if (WaitForSingleObject(cb_data.semaphore, INFINITE) == WAIT_FAILED)
        return false;
    if (LdrUnregisterDllNotification_fn(cookie) != STATUS_SUCCESS)
        return false;
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

const library_info& current_module::data() const
{
    static const library_info current((IMAGE_DOS_HEADER*)_Get_current_module_handle());
    return current;
}

bool current_module::wait() const
{
    return true;
}
