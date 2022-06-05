module;

#include <string>

export module fds.convert_to_lower;

struct to_lower_obj
{
    std::string operator()(const char* str) const;
    std::string operator()(const std::string_view str) const;

    std::wstring operator()(const wchar_t* str) const;
    std::wstring operator()(const std::wstring_view str) const;
};

export namespace fds
{
    constexpr to_lower_obj to_lower;
}
