#pragma once

#include "tier1/noncopyable.h"
#include "tier2/gui/render/backend/basic_win32.h"

#include <Windows.h>

namespace FD_TIER2(gui)
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