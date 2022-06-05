module;

#include <Windows.h>

module fds.unloader;
import fds.hooks_loader;
import fds.application_info;

// import fds.console;

namespace fds
{
    [[noreturn]] static DWORD WINAPI unload_delayed(LPVOID)
    {
        hooks_loader->stop();
        Sleep(1000);
        FreeLibraryAndExitThread(app_info->module_handle, FALSE);
    }
} // namespace fds

void fds::unload()
{
    if (app_info->module_handle == GetModuleHandle(nullptr))
    {
        hooks_loader->stop();
        PostQuitMessage(FALSE);
    }
    else
    {
        CreateThread(NULL, 0, unload_delayed, nullptr, 0, NULL);
    }
}
