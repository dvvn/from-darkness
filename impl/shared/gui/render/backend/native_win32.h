#pragma once
#include "gui/render/backend/basic_win32.h"
#include "winapi/window_info.h"

namespace fd::gui
{
class basic_native_win32_backend : public basic_win32_backend
{
    HWND window_;

  public:
    basic_native_win32_backend(HWND window);
    basic_native_win32_backend();

    win::window_info window() const;
};

using native_win32_backend = basic_native_win32_backend;
} // namespace fd::gui