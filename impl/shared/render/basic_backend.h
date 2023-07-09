#pragma once

#include "interface.h"

namespace fd
{
struct basic_backend : basic_interface
{
    virtual void destroy()   = 0;
    virtual void new_frame() = 0;
};
} // namespace fd