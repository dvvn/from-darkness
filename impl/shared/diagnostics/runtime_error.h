#pragma once

#include "exception.h"

namespace fd
{
struct runtime_error : exception
{
    using exception::exception;
};
} // namespace fd