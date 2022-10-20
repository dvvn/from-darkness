#include "backend.h"

#include <fd/hooks/helper.h>

import fd.logger.impl;
import fd.assert.impl;
import fd.system.console;

import fd.gui.context;

import fd.hooks.directx;
import fd.hooks.winapi;

using namespace fd;

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    default_assert_handler assert_callback;
    assert_handler = &assert_callback;

    system_console sys_console;

    default_logs_handler logs_callback;
    logger = &logs_callback;

    logs_callback.add([&](auto msg) {
        sys_console.write(msg);
    });

    assert_callback.add([&](auto& adata) {
        sys_console.write(parse_assert_data(adata));
    });

    const gui::context gui_ctx;

    hooks::holder all_hooks = { hooks::d3d9_reset(backend.d3d), hooks::d3d9_present(backend.d3d), hooks::wndproc(backend.handle) };

    return all_hooks.enable() ? (backend.run(), EXIT_SUCCESS) : EXIT_FAILURE;
}
