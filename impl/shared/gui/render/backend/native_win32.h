#pragma once
#include "gui/render/backend/basic_win32.h"
#include "noncopyable.h"

#include <Windows.h>

namespace fd::gui
{
class basic_native_win32_backend : public basic_win32_backend
{
    HWND window_;

  public:
    basic_native_win32_backend(HWND window);
    basic_native_win32_backend();

    HWND window() const;
};

using native_win32_backend = noncopyable_wrapper<basic_native_win32_backend>;
} // namespace fd::gui