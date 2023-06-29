#pragma once
#include "array.h"
#include "core.h"

namespace fd
{
namespace detail
{
template <size_t S>
class exception_message
{
    array<char, S - 1> buff_;

  public:
    template <size_t... I>
    constexpr exception_message(char const *msg, std::index_sequence<I...>) noexcept
        : buff_{msg[I]...}
    {
    }

    constexpr exception_message(char const *msg)
        : exception_message(msg, std::make_index_sequence<S - 1>())
    {
    }
};

} // namespace detail

struct exception
{
};

struct basic_runtime_error : exception
{
};

template <size_t S>
class runtime_error : public basic_runtime_error
{
    detail::exception_message<S> message_;

  public:
    constexpr runtime_error(char const (&msg)[S])
        : message_(msg)
    {
    }
};
}