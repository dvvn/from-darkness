#pragma once

#include "container/array.h"
#include "container/vector/small.h"
#include "noncopyable.h"

#include <Windows.h>
#include <corecrt_io.h>

#include <algorithm>
#include <cassert>

namespace fd
{
namespace detail
{
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

class system_console
#ifndef UNICODE
    : public noncopyable
#endif
{
    bool console_allocated_;
    HANDLE out_;

    template <typename T>
    struct char_buffer : small_vector<T, 1024>, noncopyable
    {
        char_buffer() = default;
    };

#ifdef UNICODE
    char_buffer<wchar_t> wchar_buffer_;

    template <size_t BufferSize = -1>
    void write_wide(char const* buff, size_t const length = BufferSize) noexcept
    {
        if constexpr (BufferSize == -1)
        {
            wchar_buffer_.assign(buff, buff + length);
            write(wchar_buffer_.data(), length);
        }
        else
        {
            array<wchar_t, BufferSize> wbuff;
            std::copy_n(buff, length, wbuff.begin());
            write(wbuff.data(), length);
        }
    }
#else

#endif

  public:
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
        write_wide(ptr, length);
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
        write_wide(in_str.data(), in_str.length());
#else
        write(in_str.data(), in_str.length());
#endif
    }

#ifdef UNICODE
    template <size_t Length>
    void write(char const (&in_str)[Length])
    {
        write_wide<Length - 1>(in_str);
    }
#endif

    void write(u8string_view in_str) const;

#ifdef UNICODE
    template <typename C>
    char_buffer<wchar_t>& get_buffer() &
    {
#ifdef _DEBUG
        static_assert(std::same_as<C, char> || std::same_as<C, wchar_t>);
#endif
        wchar_buffer_.clear();
        return wchar_buffer_;
    }
#else

#endif
    template <typename C>
    char_buffer<C> get_buffer() &&
    {
        return {};
    }

    template <typename C>
    char_buffer<C> get_buffer() const&
    {
        return {};
    }
};
} // namespace fd