module;

#include <windows.h>

export module fd.hooks.winapi;
import fd.hooks.impl;

namespace fd::hooks
{
    struct wndproc_data
    {
        WNDPROC def_;
#ifdef _DEBUG
        HWND hwnd_;
#endif
    };

    export struct wndproc : impl, private wndproc_data
    {
        ~wndproc() override;

        wndproc(HMODULE handle);
        wndproc(const char* name);
        wndproc(const wchar_t* name);
        wndproc(wndproc&& other);

        string_view name() const override;

      private:
        static LRESULT WINAPI callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    };
} // namespace fd::hooks
