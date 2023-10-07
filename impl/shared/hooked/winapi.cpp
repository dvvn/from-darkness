#include "winapi.h"
#include "basic/winapi.h"
#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd
{
template <class Backend>
class hooked_wndproc final : public basic_winapi_hook
{
    Backend* backend_;
    win32_backend_info* system_backend_info_;

  public:
    WNDPROC target() const
    {
        win32_backend_info info;
        backend_->fill(&info);
        return info.proc();
    }

    hooked_wndproc(Backend* backend, win32_backend_info* system_backend_info)
        : backend_(backend)
        , system_backend_info_(system_backend_info)
    {
    }

    template <typename Fn>
    LRESULT operator()(
        Fn& original, //
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