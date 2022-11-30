#pragma once

#include <fd/functional.h>

#include <source_location>

namespace fd
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

    struct basic_assert_handler
    {
        virtual ~basic_assert_handler()                            = default;
        virtual void operator()(const assert_data&) const noexcept = 0;
    };

    constexpr void invoke(basic_assert_handler* handler, const assert_data& data, const char*)
    {
        invoke(*handler, data);
    }

    constexpr void invoke(basic_assert_handler* handler, const assert_data& data, const bool exprResult)
    {
        if (!exprResult)
            invoke(*handler, data);
    }

    [[noreturn]] void invoke(basic_assert_handler* handler, const assert_data& data, invocable auto fn)
    {
        invoke(*handler, data);
        invoke(fn); // unreachable
    }

    extern basic_assert_handler* AssertHandler;
} // namespace fd
