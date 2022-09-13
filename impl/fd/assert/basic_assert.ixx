module;

#include <fd/utility.h>

#include <source_location>

export module fd.assert;
export import fd.callback.invoker;

export namespace fd
{
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
    };

    using assert_handler_t = basic_callback_invoker<const assert_data&>;
    assert_handler_t* assert_handler;

    constexpr void invoke(assert_handler_t* handler, const assert_data& data, const char*) noexcept
    {
        invoke(*handler, data);
    }

    constexpr void invoke(assert_handler_t* handler, const assert_data& data, const bool expr_result) noexcept
    {
        if (!expr_result)
            invoke(*handler, data);
    }

    void invoke(assert_handler_t* handler, const assert_data& data)
    {
        invoke(*handler, data);
        unreachable();
    }
} // namespace fd
