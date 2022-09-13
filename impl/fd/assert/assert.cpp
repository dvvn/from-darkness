module;

#include <Windows.h>

#include <exception>
#include <source_location>

module fd.assert.impl;
import fd.string.make;
import fd.string.utf;

using namespace fd;

template <typename... T>
static auto _Build_message(const assert_data& data, T... extra)
{
#define FIRST_PART "Assertion falied!", '\n', /**/ "File: ", location.file_name(), '\n', /**/ "Line: ", to_string(location.line()), "\n\n"
#define EXPR       "Expression: ", expression
    const auto [expression, message, location] = data;
    utf_string<wchar_t> msg;

    if (expression && message)
        msg = make_string(FIRST_PART, EXPR, "\n\n", message, extra...);
    else if (expression)
        msg = make_string(FIRST_PART, EXPR, extra...);
    else if (message)
        msg = make_string(FIRST_PART, message, extra...);
    else
        msg = make_string(FIRST_PART, extra...);

    return msg;
}

static void _Default_assert_handler(const assert_data& data)
{
    const auto [expression, message, location] = data;
#ifdef WINAPI_FAMILY
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    const auto msg       = _Build_message(data, "\n\nWould you like to interrupt execution?");
    const auto terminate = MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
    if (terminate)
        std::terminate(); // todo: unload by own function instead of terminate
#else
#pragma error not implemented
#endif

#else
#pragma error not implemented
#endif
}

default_assert_handler::default_assert_handler()
{
    data_.push_back(_Default_assert_handler);
}

void default_assert_handler::operator()(const assert_data& adata) const
{
    const lock_guard guard = mtx_;
    invoke(data_, adata);
}

wstring parse_assert_message(const assert_data& adata)
{
    return _Build_message(adata);
}
