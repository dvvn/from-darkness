#pragma once

#include <cstdint>

namespace fd
{
struct core_logger
{
    virtual ~core_logger() = default;
    // virtual void flush()                                  = 0;
    virtual void init()    = 0;
};

template <typename C>
struct abstract_logger : virtual core_logger
{
    using pointer = C const *;

    virtual void do_write(pointer msg, size_t length) = 0;
};
}