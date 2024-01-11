#pragma once

#include "container/vector/small.h"
#include "string/view.h"
#include "noncopyable.h"

#include <Windows.h>

#include <algorithm>

namespace fd
{
namespace detail
{
class system_console_mode_setter : public noncopyable
{
    int old_mode_;

  public:
    system_console_mode_setter(int descriptor, int mode);
};
} // namespace detail

class system_console : public noncopyable
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

    template <size_t BufferSize>
    void write_wide(char const* buff) noexcept
    {
        wchar_t wbuff[BufferSize];
        using std::data;
        std::copy(buff, buff + BufferSize, data(wbuff));
        write(data(wbuff), BufferSize);
    }

    void write_wide(char const* buff, size_t length) noexcept;
#else

#endif

  public:
    ~system_console();
    system_console();

    static bool exists();

    void write(wchar_t const* ptr, size_t length);
    void write(char const* ptr, size_t length);
    void write(wstring_view in_str);
    void write(string_view in_str);

#ifdef UNICODE
    template <size_t Length>
    void write(char const (&in_str)[Length])
    {
        write_wide<Length - 1>(in_str);
    }
#else
    template <size_t Length>
    void write(wchar_t const (&in_str)[Length]) = delete; // WIP
#endif

    void write(u8string_view in_str) const = delete;

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