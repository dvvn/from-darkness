#include <fd_init.h>

using namespace fd;

struct unload_handler
{
    bool ignore = false;

    ~unload_handler()
    {
        if (ignore)
            return;
        rt_modules::current->unload();
    }
};

static void _Async_impl()
{
    unload_handler unload;
    if (!fd_init_core())
        return;
    if (!rt_modules::serverBrowser.wait())
        return;
    if (!fd_init_hooks())
        return;
    unload.ignore = true;
}

static void fd_init_async()
{
    invoke(async, _Async_impl);
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH: {
        fd_init_async();
        break;
    }
    case DLL_PROCESS_DETACH: {
        fd_destroy();
        break;
    }
    }

    return TRUE;
}
