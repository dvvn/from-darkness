#pragma once

#include <boost/container/small_vector.hpp>
#include <boost/noncopyable.hpp>

#include <Windows.h>

#include <algorithm>
#include <string_view>

namespace fd
{
namespace detail
{
class system_console_mode_setter : public boost::noncopyable
{
    int old_mode_;

  public:
    system_console_mode_setter(int descriptor, int mode);
};
} // namespace detail

class system_console : public boost::noncopyable
{
    bool console_allocated_;
    HANDLE out_;

    template <typename T>
    using char_buffer = boost::container::small_vector<T, 1024>;

#ifdef UNICODE
    char_buffer<wchar_t> wchar_buffer_;

    template <size_t BufferSize>
    void write_wide(char const* buff) noexcept
    {
        wchar_t wbuff[BufferSize];
        std::copy(buff, buff + BufferSize, std::data(wbuff));
        write(std::data(wbuff), BufferSize);
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
    void write(std::wstring_view in_str);
    void write(std::string_view in_str);

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