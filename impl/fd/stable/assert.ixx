module;

#include <fd/callback.h>

#include <source_location>

export module fd.assert;
export import fd.string;

struct assert_data
{
    const char* expression;
    const char* message;
    std::source_location location;

    constexpr assert_data(const char* expr, const char* msg = nullptr, const std::source_location loc = std::source_location::current())
        : expression(expr)
        , message(msg)
        , location(loc)
    {
    }

    fd::string build_message() const;
};

FD_CALLBACK(assert_handler, const assert_data&);

constexpr bool can_invoke_assert_handler(const char*)
{
    return true;
}

constexpr bool can_invoke_assert_handler(const bool expr_result)
{
    return expr_result == false;
}

export namespace fd
{
    using ::assert_data;
    using ::assert_handler;
    using ::can_invoke_assert_handler;

    using ::fd::invoke;
} // namespace fd
