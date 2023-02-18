#pragma once

#include <fd/string.h>

namespace fd
{
struct basic_log_handler
{
    virtual ~basic_log_handler() = default;

    virtual void write(string_view msg) const  = 0;
    virtual void write(wstring_view msg) const = 0;
};

void set_log_handler(basic_log_handler* handler);
} // namespace fd