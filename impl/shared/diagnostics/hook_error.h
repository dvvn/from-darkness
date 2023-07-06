#pragma once

#include "runtime_error.h"


namespace fd
{
struct hook_error : runtime_error
{
    using runtime_error::runtime_error;

    virtual char const *status() const = 0;
    char const *what() const override;
};
} // namespace fd