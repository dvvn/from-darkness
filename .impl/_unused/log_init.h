#pragma once

#include <fd/log.h>

namespace fd::log
{
struct init_data
{
    std::string pattern   = "%H:%M:%S.%e %l : %v";
    std::size_t backtrace = 0;
};

void init(init_data&& idt = {});
void init(const init_data& idt);
}