#pragma once

#include "render/backend/basic_win32.h"

#include <Windows.h>

namespace fd
{
class own_win32_backend_data : public noncopyable
{
    friend class own_win32_backend;

    WNDCLASSEX info_;
    HWND hwnd_;

    ~own_win32_backend_data();
    own_win32_backend_data();
};

class own_win32_backend final : public own_win32_backend_data, public basic_win32_backend
{
#if 0
    update_result update(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) override
    {
        if (message == WM_SIZE)
        {
            minimized_ = wparam == SIZE_MINIMIZED;
            if (!minimized_)
                size_ = lparam;
        }

        return basic_win32_backend::update(window, message, wparam, lparam);

        // return DefWindowProc(window, message, wparam, lparam);
    }
#endif

  public:
    own_win32_backend();

    using basic_win32_backend::update;
    bool update();
    void close();

    void fill(win32_backend_info* info) const;
};

}