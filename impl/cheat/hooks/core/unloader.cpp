module;

#include <Windows.h>

module cheat.unloader;
import cheat.hooks_loader;
import cheat.application_info;

// import cheat.console;

namespace cheat
{
    [[noreturn]] static DWORD WINAPI unload_delayed(LPVOID)
    {
        hooks_loader->stop();
        Sleep(1000);
        FreeLibraryAndExitThread(app_info->module_handle, FALSE);
    }
} // namespace cheat

void cheat::unload()
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
