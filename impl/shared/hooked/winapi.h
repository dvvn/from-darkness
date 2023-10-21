#pragma once
#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd::hooked::winapi
{
template <class SystemBackend>
class wndproc final : public basic_hook_callback
{
    using stored_backend = SystemBackend*;

    stored_backend backend_;

    template <typename T>
    static auto pass_original(T& original)
    {
        if constexpr (std::copyable<T&>)
            return original;
        else
            return std::ref(original);
    }

  public:
    wndproc(stored_backend backend)
        : backend_(backend)
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

        auto [response, retval] = backend_->update(window, message, wparam, lparam);

        using enum win32_backend_update_response;
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