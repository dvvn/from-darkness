#include <fd/assert_impl.h>
#include <fd/functional.h>
#include <fd/utf_string.h>

#include <Windows.h>

#include <algorithm>
#include <mutex>
#include <source_location>

using namespace fd;

static auto _correct_file_name(const string_view fullPath)
{
#ifdef FD_ROOT_DIR
    // ReSharper disable once CppInconsistentNaming
    constexpr auto is_slash = [](const char chr) {
        switch (chr)
        {
        case '\\':
        case '/':
            return true;
        default:
            return false;
        };
    };

    constexpr string_view rootDir0 = FD_STRINGIZE(FD_ROOT_DIR);
    constexpr string_view rootDir1 = FD_CONCAT(FD_STRINGIZE(FD_ROOT_DIR), "/");
    constexpr auto rootDir         = is_slash(rootDir0.back()) ? rootDir0 : rootDir1;

    if (std::ranges::equal(rootDir, fullPath, [](auto l, auto r) {
            return l == r || is_slash(l) && is_slash(r);
        }))
        return fullPath.substr(rootDir.size());
#endif
    return fullPath;
}

template <typename... T>
static utf_string<wchar_t> _build_message(const assert_data& data, T... extra)
{
#define FIRST_PART "Assertion falied!", '\n', /**/ "File: ", _correct_file_name(location.file_name()), '\n', /**/ "Line: ", to_string(location.line()), "\n\n"
#define EXPR       "Expression: ", expression
    const auto [expression, message, location] = data;

    if (expression && message)
        return make_string(FIRST_PART, EXPR, "\n\n", message, extra...);
    if (expression)
        return make_string(FIRST_PART, EXPR, extra...);
    if (message)
        return make_string(FIRST_PART, message, extra...);
    return make_string(FIRST_PART, extra...);
}

static void _default_assert_handler(const assert_data& data)
{
#ifdef WINAPI_FAMILY
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    const auto msg       = _build_message(data, "\n\nWould you like to interrupt execution?");
    const auto terminate = MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
    if (terminate)
        invoke(std::get_terminate());
#else
#pragma error not implemented
#endif

#else
#pragma error not implemented
#endif
}

default_assert_handler::default_assert_handler()
{
    data_.emplace_back(_default_assert_handler);
}

void default_assert_handler::add(function_type fn)
{
    const std::lock_guard guard(mtx_);
    data_.emplace_back(std::move(fn));
}

void default_assert_handler::operator()(const assert_data& adata) const noexcept
{
    const std::lock_guard guard(mtx_);
    std::ranges::for_each(data_, bind_back(Invoker, adata));
}

namespace fd
{
    wstring parse_assert_data(const assert_data& adata)
    {
        return _build_message(adata);
    }

    basic_assert_handler* AssertHandler;
} // namespace fd