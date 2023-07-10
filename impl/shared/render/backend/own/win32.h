#pragma once

#include "interface_holder.h"
#include "render/backend/basic_win32.h"

namespace fd
{
struct basic_own_win32_backend : basic_win32_backend
{
    using basic_win32_backend::basic_win32_backend;

    virtual void peek()         = 0;
    virtual void close()        = 0;
    virtual bool closed() const = 0;
};

class own_win32_backend;
FD_INTERFACE_FWD(own_win32_backend, basic_own_win32_backend);
}