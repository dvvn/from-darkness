#include <fd/assert_impl.h>
#include <fd/exception.h>
#include <fd/functional.h>
#include <fd/utf_string.h>
#include <fd/utility.h>

#include <Windows.h>

namespace fd
{
static auto _correct_file_name(const string_view fullPath)
{
#ifdef FD_ROOT_DIR
    constexpr auto isSlash = [](const char chr) {
        switch (chr)
        {
        case '\\':
        case '/':
            return true;
        default:
            return false;
        };
    };

    constexpr string_view rootDir0(FD_STRINGIZE(FD_ROOT_DIR));
    constexpr string_view rootDir1(FD_CONCAT(FD_STRINGIZE(FD_ROOT_DIR), "/"));
    constexpr auto        rootDir = isSlash(rootDir0.back()) ? rootDir0 : rootDir1;

    if (rootDir.size() < fullPath.size())
    {
        const auto lPtr = rootDir.data();
        const auto rPtr = fullPath.data();
        for (size_t i = 0; i < rootDir.size(); ++i)
        {
            const auto l = lPtr[i];
            const auto r = rPtr[i];

            if (l == r)
                continue;
            if (isSlash(l) && isSlash(r))
                continue;

            return fullPath;
        }
    }
    return fullPath.substr(rootDir.size());
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

// ReSharper disable once CppInconsistentNaming
void _default_assert_handler(const assert_data& adata, const bool interrupt)
{
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    std::vector<wchar_t> msg;
    if (interrupt)
    {
        _build_message(msg, adata, '\0');
        MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_OK | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL);
        unload();
    }
    else
    {
        _build_message(msg, adata, L"\n\nWould you like to interrupt execution?", '\0');
        const auto stop = MessageBoxW(nullptr, msg.data(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
        if (stop)
            unload();
    }
#else
#error not implemented
#endif
}

wstring parse(const assert_data& adata)
{
    wstring buff;
    _build_message(buff, adata);
    return buff;
}
} // namespace fd