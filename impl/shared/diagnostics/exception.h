#pragma once

#include <exception>

namespace fd
{
inline constexpr auto default_exception_message = "Unknown exception";

#ifdef _MSC_VER
struct exception : std::exception
{
    exception(char const *message = default_exception_message)
        : std::exception(message, 0)
    {
    }
};
#else
class exception : public std::exception
{
    char const *message_;

  public:
    exception(char const *message = default_exception_message)
        : message_(message)
    {
    }

    char const *what() const override
    {
        return message_ ? message_ : default_exception_message;
    }
};
#endif
} // namespace fd