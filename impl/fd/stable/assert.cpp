module;

#include <fd/callback_impl.h>

#include <Windows.h>

#include <exception>
#include <source_location>

module fd.assert;
import fd.format;
import fd.functional;
import fd.to_char;
import fd.application_info;

using namespace fd;

using assert_handler_base = FD_CALLBACK_TYPE(assert_handler);

struct assert_handler_impl final : assert_handler_base
{
    assert_handler_impl();

    void push_back(assert_handler_base::callback_type&& callback) override
    {
        assert_handler_base::push_back(std::move(callback));

        auto rbg = this->storage_.rbegin();
        using std::swap;
        swap(*rbg, *(rbg + 1));
    }
};

FD_CALLBACK_BIND(assert_handler, assert_handler_impl);

template <typename... T>
static auto _Build_message(const assert_data& data, T... extra)
{

#define FIRST_PART "Assertion falied!", '\n', /**/ "File: ", location.file_name(), '\n', /**/ "Line: ", fd::to_string(location.line()), "\n\n"
#define EXPR       "Expression: ", expression
    const auto [expression, message, location] = data;
    string msg;

    if (expression && message)
        msg = fd::make_string(FIRST_PART, EXPR, "\n\n", message, extra...);
    else if (expression)
        msg = fd::make_string(FIRST_PART, EXPR, extra...);
    else if (message)
        msg = fd::make_string(FIRST_PART, message, extra...);
    else
        msg = fd::make_string(FIRST_PART, extra...);

    return to_char<wchar_t>(msg);
}

template <typename B>
static auto _Assert_msg(const B builder, const char* expression, const char* message)
{
    if (!expression)
        return builder(message);
    if (!message)
        return builder(expression);

    return builder(expression, " (", message, ')');
}

assert_handler_impl::assert_handler_impl()
{
    assert_handler_base::push_back([](const assert_data& data) {
        const auto [expression, message, location] = data;
#ifdef WINAPI_FAMILY
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
        const auto msg       = _Build_message(data, "\n\nWould you like to interrupt execution?");
        const auto terminate = MessageBoxW(app_info->root_window.handle, msg.c_str(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
        if (terminate)
            std::terminate(); // todo: unload by own function instead of terminate
        return true;
#else
#pragma error not implemented
#endif

#else
#pragma error not implemented
#endif
    });
}
