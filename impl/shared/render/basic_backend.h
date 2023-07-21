#pragma once

#include "object.h"

namespace fd
{
struct basic_backend : basic_object
{
    virtual void destroy()   = 0;
    virtual void new_frame() = 0;
};
} // namespace fd