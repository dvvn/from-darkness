#pragma once

#include "optional.h"
#include "render/backend/basic_win32.h"

#ifndef DefWindowProc
// ReSharper disable CppInconsistentNaming
extern LRESULT __stdcall DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern LRESULT __stdcall DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
// ReSharper restore CppInconsistentNaming
#endif

namespace fd
{
struct hooked_wndproc
{
    basic_win32_backend *backend;

    LRESULT __stdcall operator()(auto &original, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const noexcept
    {
        // todo: check are unput must be blocked before update
        // if not, always call original
        // or add extra state and check it inside update
        auto result = backend->update(window, message, wparam, lparam);
        return result.finish(
#if defined(DefWindowProc)
            DefWindowProc,
#elif defined(UNICODE)
            DefWindowProcW,
#else
            DefWindowProcA,
#endif
            original,
            window,
            message,
            wparam,
            lparam);
    }
};
} // namespace fd