#pragma once
#include <source_location>

namespace fd
{
class assert_data
{
    using source_location = std::source_location;

  public:
    const char*     expression;
    const char*     message;
    source_location location;

    constexpr assert_data(const char* expr, const char* msg = nullptr, const source_location loc = source_location::current())
        : expression(expr)
        , message(msg)
        , location(loc)
    {
    }
};
}