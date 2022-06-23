module;

#include <cctype>
#include <cwctype>
#include <string>

module fd.lower_upper;

template <class Fn, typename T>
static auto _Process(const T* ptr, const Fn fn = {})
{
    std::basic_string<T> lstr;
    for (;;)
    {
        const auto chr = *ptr++;
        if (chr == '\0')
            break;
        lstr += fn(chr);
    }
    return lstr;
}

template <class Fn, typename T, typename Tr>
static auto _Process(const std::basic_string_view<T, Tr> strv, const Fn fn = {})
{
    std::basic_string<T, Tr> lstr;
    lstr.reserve(strv.size());
    for (const auto chr : strv)
        lstr += fn(chr);
    return lstr;
}

//--------

struct to_lower_chr
{
    char operator()(const char chr) const
    {
        const auto lchr = std::tolower(static_cast<int>(chr));
        return static_cast<char>(lchr);
    }

    wchar_t operator()(const wchar_t chr) const
    {
        const auto lchr = std::towlower(static_cast<wint_t>(chr));
        return static_cast<wchar_t>(lchr);
    }
};

template <typename T>
static auto _To_lower(const T str)
{
    return _Process<to_lower_chr>(str);
}

std::string to_lower_obj::operator()(const char* str) const
{
    return _To_lower(str);
}

std::string to_lower_obj::operator()(const std::string_view str) const
{
    return _To_lower(str);
}

std::wstring to_lower_obj::operator()(const wchar_t* str) const
{
    return _To_lower(str);
}

std::wstring to_lower_obj::operator()(const std::wstring_view str) const
{
    return _To_lower(str);
}

//------

struct to_upper_chr
{
    char operator()(const char chr) const
    {
        const auto lchr = std::toupper(static_cast<int>(chr));
        return static_cast<char>(lchr);
    }

    wchar_t operator()(const wchar_t chr) const
    {
        const auto lchr = std::towupper(static_cast<wint_t>(chr));
        return static_cast<wchar_t>(lchr);
    }
};

template <typename T>
static auto _To_upper(const T str)
{
    return _Process<to_upper_chr>(str);
}

std::string to_upper_obj::operator()(const char* str) const
{
    return _To_upper(str);
}

std::string to_upper_obj::operator()(const std::string_view str) const
{
    return _To_upper(str);
}

std::wstring to_upper_obj::operator()(const wchar_t* str) const
{
    return _To_upper(str);
}

std::wstring to_upper_obj::operator()(const std::wstring_view str) const
{
    return _To_upper(str);
}
