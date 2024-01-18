#pragma once

#include "gui/render/backend/basic_win32.h"
#include "winapi/window_info.h"

#include <Windows.h>

namespace fd::gui
{
class basic_own_win32_backend;

namespace detail
{
class own_win32_backend_data : public boost::noncopyable
{
    friend class basic_own_win32_backend;

    WNDCLASSEX info_;
    HWND window_;

  protected:
    ~own_win32_backend_data();
    own_win32_backend_data();
};
} // namespace detail

class basic_own_win32_backend final : public detail::own_win32_backend_data, public basic_win32_backend
{
    bool active_;

  public:
    basic_own_win32_backend();

    bool update();
    void close();

    win::window_info window() const;
};

using own_win32_backend = basic_own_win32_backend;
} // namespace fd::gui