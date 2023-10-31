#pragma once
#include "gui/render/backend/basic_win32.h"

#include <Windows.h>

namespace fd::hooked::winapi
{
template <class SystemBackend>
class wndproc final : public basic_hook_callback
{
    using stored_backend = SystemBackend*;

    stored_backend backend_;

  public:
    wndproc(stored_backend backend)
        : backend_(backend)
    {
    }

    LRESULT operator()(
        WNDPROC const original, //
        HWND window, UINT message, WPARAM wparam, LPARAM lparam) const noexcept
    {
        // todo: check are unput must be blocked before update
        // if not, always call original
        // or add extra state and check it inside update

        // if (backend_->minimized())
        // return original(window, message, wparam, lparam);

        auto [response, retval] = backend_->update(window, message, wparam, lparam);

        using enum gui::win32_backend_update_response;
        switch (response)
        {
        case skipped:
            return original(window, message, wparam, lparam);
        case updated:
            return DefWindowProc(window, message, wparam, lparam);
        case locked:
            return retval;
        default:
            unreachable();
        }
    }
};

template <class SystemBackend>
wndproc(SystemBackend*) -> wndproc<SystemBackend>;
}