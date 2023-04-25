#include <fd/logging/default.h>

namespace fd
{
template <typename C>
class unreachable_logger final : protected virtual abstract_logger<C>
{
  protected:
    using typename abstract_logger<C>::pointer;
    using typename abstract_logger<C>::data_type;

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