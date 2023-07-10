#pragma once

#include "render/backend/basic_win32.h"

// ReSharper disable CppInconsistentNaming
#ifndef DefWindowProc
__declspec(dllimport) LRESULT __stdcall DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
__declspec(dllimport) LRESULT __stdcall DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#ifdef UNICODE
#define DefWindowProc DefWindowProcW
#else
#define DefWindowProc DefWindowProcA
#endif
#endif
// ReSharper restore CppInconsistentNaming

namespace fd
{
class hooked_wndproc final
{
    basic_win32_backend *backend_;

  public:
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
} // namespace fd