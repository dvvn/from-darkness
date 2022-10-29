module;

#include <windows.h>

export module fd.hooks.winapi;
import fd.hooks.impl;

namespace fd::hooks
{
    export class wndproc : public impl, public instance<wndproc>
    {
        WNDPROC def_;
#ifdef _DEBUG
        HWND hwnd_;
#endif
      public:
        wndproc(HWND id, function_getter target);

        static LRESULT WINAPI callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept;
    };
} // namespace fd::hooks
