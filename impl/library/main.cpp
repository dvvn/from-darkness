#include <thread>

#include <fd_init.h>

#include <Windows.h>

import fd.rt_modules;
import fd.functional;

using namespace fd;

static void _Load_fn(const std::stop_token stop, const HMODULE hModule, const LPVOID lpReserved)
{
    const auto hmodule      = static_cast<HMODULE>(lpReserved);
    const auto loading_fail = bind_front(FreeLibraryAndExitThread, hmodule, EXIT_FAILURE);

    while (!fd::rt_modules::serverbrowser.loaded())
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);

        if (stop.stop_requested())
            fd::invoke(loading_fail);
    }

    if (!fd::init(nullptr, hmodule, false))
        fd::invoke(loading_fail);
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    constexpr auto loader_thread = FD_OBJECT_GET(std::jthread, FD_UNIQUE_INDEX);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH: {
        loader_thread.construct(_Load_fn, hModule, lpReserved);
        break;
    }
    case DLL_PROCESS_DETACH: {
        loader_thread.destroy(); // send stop request and join
        hooks_loader.destroy();
        break;
    }
    }

    return TRUE;
}
