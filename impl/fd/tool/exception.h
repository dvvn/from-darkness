#pragma once
#include "array.h"
#include "basic_exception.h"
#include "string.h"
#include "vector.h"

#include "fd/core.h"

namespace fd
{
namespace detail
{
template <size_t S>
class exception_message
{
    array<char, S - 1> buff_;

  public:
    using pointer  = char const *;
    using iterator = pointer;

    template <size_t... I>
    constexpr exception_message(char const *msg, std::index_sequence<I...>) noexcept
        : buff_{msg[I]...}
    {
    }

    constexpr exception_message(char const *msg)
        : exception_message(msg, std::make_index_sequence<S - 1>())
    {
    }

    operator string_view() const
    {
        return {buff_.begin(), buff_.end()};
    }
};
} // namespace detail

template <size_t S>
class runtime_error : public basic_runtime_error
{
    detail::exception_message<S> message_;

  public:
    constexpr runtime_error(char const (&msg)[S])
        : message_(msg)
    {
    }

    string_view message() const override
    {
        return message_;
    }
};

template <size_t S>
class system_error : public basic_system_error
{
    static constexpr size_t error_message_length = 512;

    detail::exception_message<S> message_;
    mutable static_vector<char, error_message_length> error_;

  public:
    system_error(char const (&msg)[S])
        : message_(msg)
    {
    }

    system_error(uintptr_t code, char const (&msg)[S])
        : basic_system_error(code)
        , message_(msg)
    {
    }

    string_view error() const noexcept override
    {
        if (error_.empty())
        {
            if (!code())
                return "Unknown";

            char buff[error_message_length];
            auto length = format(reinterpret_cast<char **>(&buff), error_message_length);
            error_.assign(buff, buff + length);
        }
        return {error_.begin(), error_.end()};
    }

    string_view message() const override
    {
        return message_;
    }
};

}