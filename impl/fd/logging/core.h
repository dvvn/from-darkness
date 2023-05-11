#pragma once

#include <cstddef>

namespace fd
{
struct core_logger
{
    // virtual void flush()                                  = 0;
    virtual void init()    = 0;
    virtual void destroy() = 0;

  protected:
    core_logger();
};

void init_logging();
void stop_logging();

template <typename C>
class log_data;

template <typename C>
struct abstract_logger : virtual core_logger
{
    using data_type = log_data<C>;
    using pointer   = C const *;

    virtual void write_before(data_type *d)           = 0;
    virtual void do_write(pointer msg, size_t length) = 0;
    virtual void write_after(data_type *d)            = 0;
};
}