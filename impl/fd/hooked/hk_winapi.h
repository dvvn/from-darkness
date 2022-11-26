#pragma once

#include <fd/hook_impl.h>

#include <windows.h>

namespace fd::hooked
{
    class wndproc : public hook_impl, public hook_instance<wndproc>
    {
        WNDPROC def_;
        HWND hwnd_;

      public:
        wndproc(HWND id, function_getter target);

        static LRESULT WINAPI callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept;
    };
} // namespace fd::hooks
