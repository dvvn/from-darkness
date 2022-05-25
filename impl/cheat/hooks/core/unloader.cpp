module;

#include <Windows.h>

module cheat.hooks.unloader;
import cheat.hooks.loader;
import cheat.application_info;

// import cheat.console;

using namespace cheat;

[[noreturn]] static DWORD WINAPI unload_delayed(LPVOID)
{
    hooks::loader->stop();
    Sleep(1000);
    FreeLibraryAndExitThread(app_info->module_handle, FALSE);
}

void hooks::unload()
{
    if (app_info->module_handle == GetModuleHandle(nullptr))
    {
        loader->stop();
        PostQuitMessage(FALSE);
    }
    else
    {
        CreateThread(NULL, 0, unload_delayed, nullptr, 0, NULL);
    }
}
