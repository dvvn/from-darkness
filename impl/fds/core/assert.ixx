module;

#include <fds/core/callback.h>

#include <source_location>
#include <string>

export module fds.assert;

struct assert_data
{
    const char* expression;
    const char* message;
    std::source_location location;

    constexpr assert_data(const char* expr, const char* msg = nullptr, const std::source_location& loc = std::source_location::current())
        : expression(expr)
        , message(msg)
        , location(loc)
    {
    }

    std::string build_message() const;
    void system_assert() const;
};

FDS_CALLBACK(assert_handler, const assert_data&);

constexpr bool can_invoke_assert_handler(const char*)
{
    return true;
}

constexpr bool can_invoke_assert_handler(const bool expr_result)
{
    return expr_result == false;
}

export namespace fds
{
    using ::assert_data;
    using ::assert_handler;
    using ::can_invoke_assert_handler;
} // namespace fds

export namespace std
{
    // for one_instance
    using ::std::invoke;
} // namespace std
