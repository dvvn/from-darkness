#pragma once

#include "container/vector/small.h"
#include "string/static.h"
#include "string/view.h"
#include "noncopyable.h"

#include <Windows.h>
#include <corecrt_io.h>

#include <cassert>
#include <vector>

namespace fd
{
namespace detail
{
template <typename CharT, class Buff = small_vector<CharT, 512>>
class system_console_char_converter
{
    Buff buff_;

  public:
    template <class StrIt>
    system_console_char_converter(StrIt const& in)
    {
#ifdef _DEBUG
        using std::data;
        using std::size;
        auto const first = data(in);
        auto const last  = first + size(in);
#else
        using std::data;
        using std::size;
        auto const first = begin(in);
        auto const last  = end(in);
#endif
        // WIP
        buff_.assign(first, last);
    }

    basic_string_view<CharT> view() const
    {
        return {buff_.data(), buff_.size()};
    }
};

class system_console_mode_setter : public noncopyable
{
    int old_mode_;

  public:
    system_console_mode_setter(int const descriptor, int const mode)
        : old_mode_{_setmode(descriptor, mode)}
    {
        assert(old_mode_ != -1);
    }
};

} // namespace detail

class system_console : public noncopyable
{
    HANDLE out_;

    void write_raw(wchar_t const* ptr, size_t const length)
    {
        DWORD written;
        WriteConsoleW(out_, ptr, length, &written, nullptr);
        assert(written == length);

        std::ignore = this;
    }

    void write_raw(char const* ptr, size_t const length)
    {
        DWORD written;
        WriteConsoleA(out_, ptr, length, &written, nullptr);
        assert(written == length);

        std::ignore = this;
    }

  public:
    ~system_console()
    {
        FreeConsole();
    }

    system_console()
    {
        if (!exists())
        {
            if (!AllocConsole())
                return;
        }

        out_ = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    static bool exists()
    {
        return GetConsoleWindow() != INVALID_HANDLE_VALUE;
    }

    void write(wstring_view const in_str)
    {
#ifdef UNICODE
        write_raw(in_str.data(), in_str.length());
#else
        detail::system_console_char_converter<char8_t> const buff{in_str};
        write(buff.view());
#endif
    }

    /*template <size_t Length>
    void write(wchar_t const (&in_str)[Length])
    {
#ifdef UNICODE
        write_raw(in_str, Length - 1);
#else
        detail::system_console_char_converter<char8_t> const buff{in_str};
        write(buff.view());
#endif
    }*/

    void write(string_view const in_str)
    {
#ifndef UNICODE
        write_raw(in_str.data(), in_str.length());
#else
                detail::system_console_char_converter<wchar_t> const buff{in_str};
        write(buff.view());
#endif
    }

    template <size_t Length>
    void write(char const (&in_str)[Length])
    {
        std::vector<int> xx;
#ifdef UNICODE
        write_raw(in_str, Length - 1);
#else
        constant_wstring const buff;
        write_raw(buff.data(), buff.length());
#endif
    }

    void write(u8string_view const in_str) const;
};
} // namespace fd