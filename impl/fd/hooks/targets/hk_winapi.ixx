module;

#include <windows.h>

export module fd.hooks.winapi;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct wndproc : impl
    {
        ~wndproc() override;

        wndproc(HMODULE handle);
        wndproc(PTCHAR name);
        wndproc(wndproc&& other);

        string_view name() const override;

      private:
        static LRESULT WINAPI callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    };
} // namespace fd::hooks
