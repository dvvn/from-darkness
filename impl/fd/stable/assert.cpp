module;

#include <fd/callback_impl.h>

/* #if !defined(NDEBUG)
#include <assert.h>
#endif */

#include <functional>
#include <source_location>

//
module fd.assert;
//--

#pragma warning(disable : 5244)
#if !defined(NDEBUG)
#include <assert.h>
#else
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#endif

using selected_assert_callback = fd::callback<const assert_data&>;

struct assert_handler_impl : selected_assert_callback
{
    assert_handler_impl();

    void invoke(const assert_data& data) const override
    {
        selected_assert_callback::invoke(data);
        std::terminate();
    }
};

FD_CALLBACK_BIND(assert_handler, assert_handler_impl);

fd::string assert_data::build_message() const
{
#define FIRST_PART "Assertion falied!", '\n', /**/ "File: ", location.file_name(), '\n', /**/ "Line: ", fd::to_string(location.line()), "\n\n"
#define EXPR       "Expression: ", expression

    if (expression && message)
        return fd::make_string(FIRST_PART, EXPR, "\n\n", message);
    if (expression)
        return fd::make_string(FIRST_PART, EXPR);
    if (message)
        return fd::make_string(FIRST_PART, message);

    return fd::make_string(FIRST_PART);
}

template <typename B>
static auto _Assert_msg(const B builder, const char* expression, const char* message)
{
    if (!expression)
        return builder(message);
    if (!message)
        return builder(expression);

    return builder(expression, "( ", message, ')');
}

assert_handler_impl::assert_handler_impl()
{
    selected_assert_callback::append([](const assert_data& data) {
        const auto [expression, message, location] = data;
#if defined(_MSC_VER)
        auto builder = std::bind_front(fd::make_string, std::in_place_type<wchar_t>);
        _wassert(_Assert_msg(builder, expression, message).c_str(), builder(location.file_name()).c_str(), location.line());
#elif defined(__GNUC__)
        constexpr auto builder = std::bind_front(fd::make_string, std::in_place_type<char>);
        __assert_fail(_Assert_msg(builder, expression, message).c_str(), location.file_name(), location.line(), location.function_name());
#else
#error not implemented
#endif
    });
}
