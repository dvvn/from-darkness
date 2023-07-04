#pragma once

#include <exception>

namespace fd
{
#ifdef _MSC_VER
struct exception : std::exception
{
    exception(char const *message = nullptr)
        : std::exception(message, 0)
    {
    }
};
#else
class exception : public std::exception
{
    char const *message_;

  public:
    exception(char const *message = nullptr)
        : message_(message)
    {
    }

    char const *what() const override
    {
        return message_ ? message_ : "Unknown exception";
    }
};
#endif
} // namespace fd