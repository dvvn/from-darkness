#pragma once

#include "basic_win32.h"
//
#include "noncopyable.h"

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
    bool active_;

  public:
    own_win32_backend();

    bool update();
    void close();

    win32_backend_info info() const;
};
}