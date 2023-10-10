#pragma once
#include "hook/basic_callback.h"
#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd
{
template <class SystemBackend>
class hooked_wndproc final : public basic_hook_callback
{
    SystemBackend* backend_;
    win32_backend_info* backend_info_;

  public:
    WNDPROC target() const
    {
        return backend_info_->proc();
    }

    hooked_wndproc(SystemBackend* backend, win32_backend_info* system_backend_info)
        : backend_(backend)
        , backend_info_(system_backend_info)
    {
    }

    LRESULT operator()(
        auto& original, //
        HWND window, UINT message, WPARAM wparam, LPARAM lparam) const noexcept
    {
        // todo: check are unput must be blocked before update
        // if not, always call original
        // or add extra state and check it inside update

        // if (backend_->minimized())
        // return original(window, message, wparam, lparam);
        using enum win32_backend_update_response;
        return backend_->update(window, message, wparam, lparam)(
            make_win32_backend_update_response<skipped>(original),
            make_win32_backend_update_response<updated>(DefWindowProc),
            make_win32_backend_update_response<locked>(win32_backend_update_unchanged()));
    }
};
}