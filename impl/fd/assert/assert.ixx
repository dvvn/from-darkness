module;

#include <fd/callback.h>

#include <source_location>

export module fd.assert;
import fd.string;

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

    fd::wstring build_message() const;
};

// constexpr auto assert_handler = fd::instance_of<fd::abstract_callback<void, const assert_data&>, 1337>;

FD_CALLBACK(assert_handler, void, const assert_data&);

// using assert_handler_t = fd::abstract_callback<bool, const assert_data&>;
// FD_OBJECT(assert_handler, assert_handler_t)

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
} // namespace fd
