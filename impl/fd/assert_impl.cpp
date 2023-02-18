#include <fd/assert_impl.h>
#include <fd/format.h>
#include <fd/functional.h>
#include <fd/utility.h>

#include <Windows.h>

namespace fd
{
static auto _correct_file_name(const string_view fullPath)
{
#ifdef FD_WORK_DIR
    constexpr string_view rootDir = (FD_CONCAT(FD_STRINGIZE(FD_ROOT_DIR), "/"));

    if (rootDir.size() < fullPath.size())
    {
        constexpr auto isSlash = [](const char chr) {
            return chr == '\\' || chr == '/';
        };
        constexpr auto checkEx = [](const char c1, const char c2) {
            return c1 == c2 || (isSlash(c1) && isSlash(c2));
        };

        const auto checkedPart = fullPath.substr(rootDir.size());
        if (checkedPart.contains('\\') ? std::ranges::equal(checkedPart, rootDir, checkEx) : checkedPart == rootDir)
            return fullPath.substr(0, rootDir.size());
    }
#endif

    return fullPath;
}

template <class Buff, typename... T>
static auto _build_message(Buff& buffer, const assert_data& data, T... extra)
{
#define FIRST_PART L"Assertion falied!", '\n', /**/ L"File: ", _correct_file_name(location.file_name()), '\n', /**/ "Line: ", to_string(location.line()), "\n\n"
#define EXPR       L"Expression: ", expression
    const auto [expression, message, location] = data;

    if (expression && message)
        write_string(buffer, FIRST_PART, EXPR, "\n\n", message, extra...);
    else if (expression)
        write_string(buffer, FIRST_PART, EXPR, extra...);
    else if (message)
        write_string(buffer, FIRST_PART, message, extra...);
    else
        write_string(buffer, FIRST_PART, extra...);
}

void _default_assert_handler::run(const assert_data& adata) const noexcept
{
    std::vector<wchar_t> msg;
    _build_message(msg, adata, L"\n\nWould you like to interrupt execution?", '\0');
    bool stop;
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    stop = MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
#else
    stop = true;
#endif
    if (stop)
        terminate();
}

void _default_assert_handler::run_panic(const assert_data& adata) const noexcept
{
    std::vector<wchar_t> msg;
    _build_message(msg, adata, '\0');
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_OK | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL);
#else

#endif
    terminate();
}

wstring parse(const assert_data& adata)
{
    wstring buff;
    _build_message(buff, adata);
    return buff;
}
} // namespace fd