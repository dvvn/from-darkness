#pragma once

#include <fd/object.h>
#include <fd/utility.h>

#include <Windows.h>

import fd.hooks_loader;
import fd.application_data;
import fd.rt_modules;
#ifdef _DEBUG
import fd.assert;
import fd.logger;
import fd.system_console;
#endif
#ifndef FD_GUI_TEST
import fd.async;
#endif

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

#ifdef FD_GUI_TEST
struct IDirect3DDevice9;
#define CSGO_HOOKS
#else
#define CSGO_HOOKS , vgui_surface_lock_cursor
#endif

PREPARE_HOOKS(_Store_hooks, wndproc, IDirect3DDevice9_present, IDirect3DDevice9_reset CSGO_HOOKS);

namespace fd
{
    inline void _Init(const HWND hwnd, const HMODULE hmodule)
    {
#if defined(_DEBUG) || defined(FD_GUI_TEST)
        logger.append(system_console_writer);
        assert_handler->push_back([](const assert_data& data) {
            invoke(logger, data.build_message());
        });
#endif
        app_info.construct(hwnd, hmodule);
    }

    inline bool _Init_hooks()
    {
        _Store_hooks(*hooks_loader);
        return hooks_loader->enable();
    }

#ifdef FD_GUI_TEST
    inline bool init(const HWND hwnd, const HMODULE hmodule, IDirect3DDevice9* const d3d_created)
    {
        _Init(hwnd, hmodule);
        FD_OBJECT_GET(IDirect3DDevice9*).construct(d3d_created);
        rt_modules::current->log_class_info<IDirect3DDevice9>(d3d_created);
        return _Init_hooks();
    }
#else
    template <class T>
    inline bool _Wait_for_game(const T& run)
    {
        for (;;)
        {
            if (!run)
                return false;
            if (rt_modules::serverbrowser.loaded())
                return true;

            thread_sleep(100);
        }
    }

    inline void init(const HWND hwnd, const HMODULE hmodule)
    {
        invoke(async, [=](const auto& stop) {
            _Init(hwnd, hmodule);
            if (!_Wait_for_game(stop) || !_Init_hooks())
                app_info->unload();
        });
    }
#endif
    inline void destroy()
    {
#ifndef FD_GUI_TEST
        async.destroy();
#endif
        hooks_loader.destroy();
    }

} // namespace fd
