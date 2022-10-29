module;

#include <windows.h>

export module fd.hooks.winapi;
import fd.hooks.impl;

namespace fd::hooks
{

    export class wndproc : public impl
    {
        WNDPROC def_;
#ifdef _DEBUG
        HWND hwnd_;
#endif
      public:
        wndproc(HWND id, WNDPROC target);
        wndproc(wndproc&& other);

      private:
        struct wndproc_data
        {
            HWND window;
            UINT message;
            WPARAM w_param;
            LPARAM l_param;
        };

        static LRESULT WINAPI callback(wndproc_data data);
    };
} // namespace fd::hooks
