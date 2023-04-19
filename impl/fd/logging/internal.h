#pragma once

namespace fd
{
template <typename C>
class log_data;

template <typename C>
struct internal_logger
{
    using data_type = log_data<C>;

    virtual ~internal_logger() = default;

    virtual void write_before(data_type *d) = 0;
    virtual void write_after(data_type *d)  = 0;
};
}