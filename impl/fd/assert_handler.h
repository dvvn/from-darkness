#pragma once

#include <source_location>

namespace fd
{
    struct assert_data
    {
        const char*          expression;
        const char*          message;
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
        virtual ~basic_assert_handler() = default;

        virtual void run(const assert_data& adata) const noexcept       = 0;
        virtual void run_panic(const assert_data& adata) const noexcept = 0;

      protected:
        static void set(basic_assert_handler* handler);
    };

    void run_assert(const assert_data& data, const char*);
    void run_assert(const assert_data& data, bool exprResult);

    [[noreturn]] void run_panic_assert(const assert_data& data);
} // namespace fd