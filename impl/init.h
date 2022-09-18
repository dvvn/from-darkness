#pragma once

#include <Windows.h>

#include <tuple>

struct IDirect3DDevice9;

#ifdef _DEBUG
import fd.logger.impl;
import fd.assert.impl;
import fd.system.console;
#endif

import fd.gui.context;

import fd.hooks.directx;
import fd.hooks.winapi;

namespace fd
{
    template <typename... H>
    bool load_hooks(std::tuple<H...>& hooks)
    {
        return (std::get<H>(hooks).enable() && ...);
    }

    template <typename... H>
    bool disable_hooks(std::tuple<H...>& hooks)
    {
        return (std::get<H>(hooks).disable() && ...);
    }

    struct init_data
    {
        IDirect3DDevice9* d3d;
        HMODULE handle;
    };

    template <typename Fn>
    BOOL init(const init_data data, Fn app_runner)
    {
        if (!data.d3d || !data.handle)
            return FALSE;

#ifdef _DEBUG
        default_assert_handler assert_callback;
        assert_handler = &assert_callback;

        system_console sys_console;

        default_logs_handler logs_callback;
        logger = &logs_callback;

        logs_callback.add([&](auto msg) {
            sys_console.write(msg);
        });

        assert_callback.add([&](const assert_data_parsed data_parsed) {
            sys_console.write(data_parsed);
        });
#endif

        // FD_OBJECT_GET(IDirect3DDevice9*).construct(g_pd3dDevice.Get());
        // rt_modules::current->log_class_info(g_pd3dDevice.Get());

        const gui::context gui_ctx;

        std::tuple core_hooks = { hooks::d3d9_reset(data.d3d), hooks::d3d9_present(data.d3d), hooks::wndproc(data.handle) };

        if (!load_hooks(core_hooks))
            return TRUE;

        // todo: csgo hooks here

        return app_runner();
    }
} // namespace fd
