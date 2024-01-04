#pragma once

#include "container/vector/small.h"
#include "string/static.h"
#include "type_traits/conditional.h"
#include "noncopyable.h"

#include <Windows.h>
#include <corecrt_io.h>

#include <cassert>

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
    bool console_allocated_;
    HANDLE out_;

#ifdef UNICODE
    template <size_t BufferSize = -1>
    void write_wide(char const* buff, size_t const length)
    {
        using buff_t = conditional_t<BufferSize == -1, small_vector<wchar_t, 512>, basic_static_string<wchar_t, BufferSize>>;
        buff_t const wbuff{buff, buff + length};
        return write(wbuff.data(), wbuff.size());
    }
#else

#endif

  public:
    using native_char_type = wchar_t;

    ~system_console()
    {
        if (console_allocated_)
            FreeConsole();
    }

    system_console()
        : console_allocated_{false}
    {
        if (!exists())
        {
            if (!AllocConsole())
            {
                assert(0 && "AllocConsole error!");
                return;
            }
            console_allocated_ = true;
        }

        out_ = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    static bool exists()
    {
        return GetConsoleWindow() != INVALID_HANDLE_VALUE;
    }

    void write(wchar_t const* ptr, size_t const length)
    {
#ifdef UNICODE
        DWORD written;
        WriteConsoleW(out_, ptr, length, &written, nullptr);
        assert(written == length);
#else
        FIXME
#endif
    }

    void write(char const* ptr, size_t const length)
    {
#ifdef UNICODE
        return write_wide(ptr, length);
#else
        DWORD written;
        WriteConsoleA(out_, ptr, length, &written, nullptr);
        assert(written == length);
#endif
    }

    void write(wstring_view const in_str)
    {
#ifdef UNICODE
        write(in_str.data(), in_str.length());
#else
        FIXME
#endif
    }

    void write(string_view const in_str)
    {
#ifdef UNICODE
        write_wide(in_str.data(), in_str.size());
#else
        FIXME
#endif
    }

    template <size_t Length>
    void write(char const (&in_str)[Length])
    {
#ifdef UNICODE
        write_wide<Length - 1>(in_str, Length - 1);
#else
        FIXME
#endif
    }

    void write(u8string_view const in_str) const;
};
} // namespace fd