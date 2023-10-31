#pragma once
#include "noncopyable.h"
#include "gui/render/backend/basic_win32.h"

namespace fd::gui
{
class native_win32_backend final : public basic_win32_backend, public noncopyable
{
    HWND window_;

  public:
    native_win32_backend();
    native_win32_backend(HWND window);

    win32_window_info info() const;
};
} // namespace fd