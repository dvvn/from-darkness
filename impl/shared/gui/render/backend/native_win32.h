#pragma once
#include "noncopyable.h"
#include "gui/render/backend/basic_win32.h"
#include "winapi/window_info.h"

namespace fd::gui
{
class native_win32_backend final : public basic_win32_backend, public noncopyable
{
    HWND window_;

  public:
    native_win32_backend();
    native_win32_backend(HWND window);

    HWND window() const;
};
} // namespace fd