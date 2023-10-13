﻿#pragma once
#include "basic_win32.h"
//
#include "noncopyable.h"

namespace fd
{
class native_win32_backend final : public basic_win32_backend, public noncopyable
{
    HWND window_;

    static HWND find_game_window();

  public:
    native_win32_backend(HWND window = find_game_window());

    void fill(win32_window_info* backend_info) const;
};
} // namespace fd