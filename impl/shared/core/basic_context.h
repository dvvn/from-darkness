#pragma once

#include "preprocessor/random.h"
#include "logger.h"
#include "system_console.h"

namespace fd
{
template <class T, bool Store = std::is_trivially_destructible_v<T> && !std::is_empty_v<T>>
class basic_context_data_holder;

template <class T>
class basic_context_data_holder<T, true>
{
  public:
    using value_type      = T;
    using reference       = value_type&;
    using const_reference = value_type const&;

  private:
    value_type data_;

  public:
    reference get()
    {
        return data_;
    }

    const_reference get() const
    {
        return data_;
    }
};

template <class T>
class basic_context_data_holder<T, false>
{
    void debug_check() const
    {
#ifdef _DEBUG
        static auto const src = this;
        assert(src == this);
#endif
    }

  public:
    using value_type = T;

    value_type get()
    {
        debug_check();
        return {};
    }

    value_type get() const
    {
        debug_check();
        return {};
    }
};

class basic_context
{
  protected:
    using logger_type       = basic_context_data_holder<basic_logger<system_console>>;
    using empty_logger_type = basic_context_data_holder<empty_logger>;

#ifdef _DEBUG
    union
    {
        logger_type logger;
        logger_type debug_logger;
    };
#else
    logger_type logger;
    [[no_unique_address]] //
    empty_logger_type debug_logger;
#endif

  public:
    basic_context();
};

bool attach_context();
} // namespace fd