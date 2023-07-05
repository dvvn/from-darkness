#pragma once

#include "noncopyable.h"
#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd
{
class own_window_info : public noncopyable
{
    WNDCLASSEX info_;
    HWND hwnd_;

  protected:
    ~own_window_info();

  public:
    own_window_info(LPCTSTR name, HMODULE handle, HWND parent);

    WNDPROC wndproc() const;
    HWND handle() const;
};

class win32_backend_own final : own_window_info, public basic_win32_backend
{
    struct window_params
    {
        bool minimized;
        UINT w, h;
    } params_;

    update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) override;

  public:
    ~win32_backend_own();
    win32_backend_own(HWND parent = nullptr);

    window_params *peek();

    WNDPROC proc() const override;
    HWND id() const override;
};
}