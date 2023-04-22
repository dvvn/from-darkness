#include <fd/logging/default.h>

namespace fd
{
template <typename C>
struct unreachable_logger final : virtual abstract_logger<C>, virtual internal_logger<C>
{
    using typename abstract_logger<C>::pointer;
    using typename internal_logger<C>::data_type;

    void init() override
    {
        std::unreachable();
    }

    void destroy() override
    {
        std::unreachable();
    }

    void do_write(pointer msg, size_t length) override
    {
        std::unreachable();
    }

    void write_before(data_type *d) override
    {
        std::unreachable();
    }

    void write_after(data_type *d) override
    {
        std::unreachable();
    }
};

static logger_impl_wrapped<default_logger_t, unreachable_logger> default_logger_impl;

default_logger_p default_logger = &default_logger_impl;
}