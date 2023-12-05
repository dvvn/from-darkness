#pragma once

#include "noncopyable.h"
#include "gui/render/backend/basic_win32.h"

#include <Windows.h>

namespace fd::gui
{
class own_win32_backend_data : public noncopyable
{
    friend class own_win32_backend;

    WNDCLASSEX info_;
    HWND window_;

  public:
    ~own_win32_backend_data();
    own_win32_backend_data();
};

class own_win32_backend final : public own_win32_backend_data, public basic_win32_backend
{
    bool active_;

  public:
    own_win32_backend();

    bool update();
    void close();

    HWND window() const;
};
}