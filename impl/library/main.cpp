#include <fd_init.h>

#include <windows.h>

import fd.functional.lazy_invoke;

using namespace fd;

PREPARE_HOOKS(fd_init_hooks, wndproc, IDirect3DDevice9_present, IDirect3DDevice9_reset, vgui_surface_lock_cursor);

static auto _Unload()
{
    rt_modules::current->unload();
}

static void _Async_impl()
{
    lazy_invoke unload_handler(_Unload);
    if (!fd_init_core())
        return;
    if (!rt_modules::serverBrowser.wait())
        return;
    if (!fd_init_hooks())
        return;
    unload_handler.reset();
}

static void fd_init_async()
{
    invoke(FD_OBJECT_GET(async), _Async_impl, async_tags::simple);
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
