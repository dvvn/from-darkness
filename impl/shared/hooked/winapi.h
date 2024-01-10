#pragma once

#include "hook/proxy.h"

#include <Windows.h>

namespace fd::hooked::winapi
{
template <class SystemBackend>
class wndproc final : public basic_hook_callback
{
    SystemBackend* backend_;

  public:
    wndproc(SystemBackend* backend)
        : backend_(backend)
    {
    }

    LRESULT operator()(
        auto const& original, //
        HWND window, UINT message, WPARAM wparam, LPARAM lparam) const noexcept
    {
#if 0
        if (menu->closed())
        {
            switch (message)
            {
            case WM_MOUSEMOVE:
            case WM_NCMOUSEMOVE:
                //
            case WM_MOUSELEAVE:
            case WM_NCMOUSELEAVE:
                //
            case WM_MOUSEWHEEL:
            case WM_MOUSEHWHEEL:
                //
            case WM_SETCURSOR:
                return original(window, message, wparam, lparam);
            }
        }
#endif
        auto const result = backend_->update(window, message, wparam, lparam);
        return result.retval_or_default_or_original(window, message, wparam, lparam, original);
    }
};
} // namespace fd::hooked::winapi
