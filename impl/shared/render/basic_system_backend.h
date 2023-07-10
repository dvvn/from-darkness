#pragma once

#include "basic_backend.h"

namespace fd
{
struct basic_system_backend : basic_backend
{
    virtual bool minimized() const = 0;
};
}