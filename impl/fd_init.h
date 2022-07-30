#pragma once

#include <fd/object.h>
#include <fd/utility.h>

#include <Windows.h>

import fd.hooks_loader;
import fd.logger;
import fd.system_console;
import fd.application_info;

namespace fd
{
    struct hook_base;
}

template <class T, class L>
void _Store_hook(L& loader)
{
    const auto ptr      = &FD_OBJECT_GET(T*);
    // UNSAFE
    //  ok in debug mode
    // ??? in release mode
    const auto base_ptr = reinterpret_cast<fd::hook_base*>(ptr);
    loader.store(base_ptr);
}

#define _HOOK_NAME(_N_) hk_##_N_

#define _DECLARE_HOOK(_NAME_) struct _HOOK_NAME(_NAME_);
#define _STORE_HOOK(_NAME_)   _Store_hook<_HOOK_NAME(_NAME_)>(loader);

#define PREPARE_HOOKS(_FN_NAME_, ...)       \
    FOR_EACH(_DECLARE_HOOK, __VA_ARGS__);   \
    template <class L>                      \
    void _FN_NAME_(L& loader)               \
    {                                       \
        FOR_EACH(_STORE_HOOK, __VA_ARGS__); \
    }

PREPARE_HOOKS(_Store_basic_hooks, wndproc, IDirect3DDevice9_present, IDirect3DDevice9_reset);
PREPARE_HOOKS(_Store_csgo_hooks, vgui_surface_lock_cursor);

namespace fd
{
    inline void init(const HWND hwnd, const HMODULE hmodule)
    {
        logger.append(system_console_writer);
        app_info.construct(hwnd, hmodule);
    }

    inline bool init_hooks(const bool gui_only)
    {
        auto& loader = *hooks_loader;
        _Store_basic_hooks(loader);
        if (!gui_only)
            _Store_csgo_hooks(loader);
        return loader.init();
    }

} // namespace fd
