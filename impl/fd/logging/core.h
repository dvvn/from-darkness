#pragma once
#include <fd/logging/levels.h>

namespace fd
{
struct abstract_logger
{
    friend void init_logging();

    virtual ~abstract_logger()                            = default;
    virtual void write(const char *msg, size_t length)    = 0;
    virtual void write(const wchar_t *msg, size_t length) = 0;
    // virtual void flush()                                  = 0;
  protected:
    virtual void init() = 0;
};

template <log_level Level>
struct dummy_logger
{
    template <log_level CurrLevel>
    void write(...) requires(CurrLevel > Level)
    {
        static_assert(CurrLevel > log_level::off);
        (void)this;
    }
};

template <log_level Level>
struct basic_logger : virtual abstract_logger, dummy_logger<Level>
{
};

}