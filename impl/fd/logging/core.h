#pragma once
#include <fd/logging/levels.h>

namespace fd
{
struct abstract_logger
{
    using pointer  = char const *;
    using wpointer = wchar_t const *;

    virtual ~abstract_logger()                      = default;
    virtual void write(pointer msg, size_t length)  = 0;
    virtual void write(wpointer msg, size_t length) = 0;
    // virtual void flush()                                  = 0;

    virtual void init() = 0;
};

template <log_level Level>
struct basic_logger : virtual abstract_logger
{
    template <log_level CurrLevel>
    void write(...) requires(!(CurrLevel & Level))
    {
        (void)this;
    }
};
}