#pragma once

#include <fd/object.h>

#include <Windows.h>

import fd.hooks_loader;
import fd.logger;
import fd.system_console;
import fd.application_info;

struct hk_wndproc;
struct hk_IDirect3DDevice9_present;
struct hk_IDirect3DDevice9_reset;

namespace fd
{
    inline bool init(const HWND hwnd, const HMODULE hmodule)
    {
        app_info.construct(hwnd, hmodule);
        logger.append(system_console_writer);
        return hooks_loader->load<hk_wndproc, hk_IDirect3DDevice9_present, hk_IDirect3DDevice9_reset>();
    }
} // namespace fd
