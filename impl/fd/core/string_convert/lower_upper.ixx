module;

#include <string>

export module fd.lower_upper;

struct to_lower_obj
{
    std::string operator()(const char* str) const;
    std::string operator()(const std::string_view str) const;

    std::wstring operator()(const wchar_t* str) const;
    std::wstring operator()(const std::wstring_view str) const;
};

struct to_upper_obj
{
    std::string operator()(const char* str) const;
    std::string operator()(const std::string_view str) const;

    std::wstring operator()(const wchar_t* str) const;
    std::wstring operator()(const std::wstring_view str) const;
};

export namespace fd
{
    constexpr to_lower_obj to_lower;
    constexpr to_upper_obj to_upper;
} // namespace fd
