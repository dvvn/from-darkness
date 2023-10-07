#pragma once
#include "exception.h"

namespace fd
{
struct logic_error : exception
{
    using exception::exception;
};
}