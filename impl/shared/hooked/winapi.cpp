#include "winapi.h"
#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd
{
class hooked_wndproc final : public basic_winapi_hook
{
    basic_win32_backend *backend_;

  public:
    auto target() const
    {
        return backend_->proc();
    }

    hooked_wndproc(basic_win32_backend *backend)
        : backend_(backend)
    {
    }

    template <typename Fn>
    LRESULT operator()(Fn &original, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const noexcept
    {
        // todo: check are unput must be blocked before update
        // if not, always call original
        // or add extra state and check it inside update

        // if (backend_->minimized())
        // return original(window, message, wparam, lparam);

        auto result = backend_->update(window, message, wparam, lparam);
        return result.finish(original, DefWindowProc, window, message, wparam, lparam);
    }
};

FD_HOOK_IMPL(hooked_wndproc);
}