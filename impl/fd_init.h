#pragma once

#include <fd/object.h>
#include <fd/utility.h>

#include <Windows.h>

import fd.hooks_loader;
import fd.rt_modules;
import fd.assert;
import fd.logger;
import fd.system.console;
import fd.async;

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
// todo: remove when compiler fix errors
struct custom_atomic_bool;
#define CSGO_HOOKS , vgui_surface_lock_cursor
#endif

PREPARE_HOOKS(_Store_hooks, wndproc, IDirect3DDevice9_present, IDirect3DDevice9_reset CSGO_HOOKS);

namespace fd
{
    inline bool fd_init_core()
    {
#if defined(_DEBUG) || defined(FD_GUI_TEST)
        logger->append([](const auto str) {
            invoke(system_console_writer, str);
        });
#endif
#ifdef _DEBUG
        assert_handler->push_back([](const assert_data& data) {
            invoke(system_console_writer, data.build_message());
        });
#endif
        return true;
    }

    inline bool fd_init_hooks()
    {
        _Store_hooks(*hooks_loader);
        return hooks_loader->enable();
    }

    inline void fd_destroy()
    {
        async.destroy();
        hooks_loader.destroy();
    }

} // namespace fd
