#ifdef _DEBUG
#include "log.h"
#include "container/vector/small.h"

#include <boost/nowide/iostream.hpp>

#include <fmt/chrono.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <cassert>
#include <iostream>
#include <mutex>

namespace fd::inline debug
{
static thread_local small_vector<uint32_t, 512> tmp_buff;

template <typename T>
static auto &get_buff()
{
    static_assert(sizeof(T) <= sizeof(uint32_t));
    static_assert(std::is_trivial_v<T>);
    tmp_buff.clear();
    return reinterpret_cast<small_vector<T, 512 * (sizeof(uint32_t) / sizeof(T))> &>(tmp_buff);
}

template <typename T>
static void do_log(fmt::basic_string_view<T> text, std::basic_ostream<T> *out)
{
    auto time = std::chrono::system_clock::now().time_since_epoch();

    auto &buff = get_buff<T>();
    auto it    = std::back_inserter(buff);

    format_to(it, "[{:%H:%M:%S}]", time);
    buff.push_back(' ');
    std::copy(text.begin(), text.end(), it);
    buff.push_back('\n');

    out->write(buff.data(), buff.size());
    out->flush();
}

template <typename T>
static void do_log(fmt::basic_string_view<T> fmt, auto &fmt_args, std::basic_ostream<T> *out)
{
    auto time = std::chrono::system_clock::now().time_since_epoch();

    auto &buff = get_buff<T>();
    auto it    = std::back_inserter(buff);

    format_to(it, "[{:%H:%M:%S}]", time);
    buff.push_back(' ');
    vformat_to(it, fmt, fmt_args);
    buff.push_back('\n');

    out->write(buff.data(), buff.size());
    out->flush();
}

log_activator::~log_activator()
{
    if (orig_mode_ == -1)
        return;
    std::ios::sync_with_stdio(orig_sync_);
    auto mode_restored = _setmode(_fileno(stdout), orig_mode_) != -1;
    do_log<char>("Stopped", mode_restored ? &std::cout : &boost::nowide::cout);
}

log_activator::log_activator() noexcept
{
    orig_sync_ = std::ios::sync_with_stdio(false);
    do_log<char>("Started", &std::cout);
    orig_mode_ = _setmode(_fileno(stdout), _O_WTEXT);
    assert(orig_mode_ != -1);
}

void log(fmt::string_view fmt, fmt::format_args fmt_args, std::ostream *out)
{
    // assert(out != &std::cout);
    do_log(fmt, fmt_args, out ? out : &boost::nowide::cout);
}

void log(fmt::wstring_view fmt, fmt::wformat_args fmt_args, std::wostream *out)
{
    do_log(fmt, fmt_args, out ? out : &std::wcout);
}
} // namespace fd::debug
#endif