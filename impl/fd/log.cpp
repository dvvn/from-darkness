#include "log.h"
#include "tool/vector.h"

#include <boost/nowide/iostream.hpp>

#include <fmt/chrono.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <mutex>

using fmt::basic_string_view;
using fmt::format_to;
using fmt::string_view;
using fmt::vformat_to;
using fmt::wstring_view;

constexpr auto sync_with_stdio = std::ios::sync_with_stdio;

namespace fd
{
template <typename T>
static void do_log(basic_string_view<T> text, std::basic_ostream<T> *out)
{
    auto time = std::chrono::system_clock::now().time_since_epoch();

    small_vector<T, 128> buff;
    auto it = std::back_inserter(buff);

    format_to(it, "[{:%H:%M:%S}]", time);
    buff.push_back(' ');
    std::copy(text.begin(), text.end(), it);
    buff.push_back('\n');

    out->write(buff.data(), buff.size());
    out->flush();
}

template <typename T>
static void do_log(basic_string_view<T> fmt, auto &fmt_args, std::basic_ostream<T> *out)
{
    auto time = std::chrono::system_clock::now().time_since_epoch();

    small_vector<T, 512> buff;
    auto it = std::back_inserter(buff);

    format_to(it, "[{:%H:%M:%S}]", time);
    buff.push_back(' ');
    vformat_to(it, fmt, fmt_args);
    buff.push_back('\n');

    out->write(buff.data(), buff.size());
    out->flush();
}

logging_activator::~logging_activator()
{
    if (!operator bool())
        return;
    sync_with_stdio(prev_sync_);
    auto mode_restored = _setmode(_fileno(stdout), prev_mode_) != -1;
    do_log<char>("Stopped", mode_restored ? &std::cout : &boost::nowide::cout);
}

logging_activator::logging_activator()
{
    prev_sync_ = sync_with_stdio(false);
    do_log<char>("Started", &std::cout);
    prev_mode_ = _setmode(_fileno(stdout), _O_WTEXT);
}

logging_activator::operator bool() const
{
    return prev_mode_ != -1;
}

void log(string_view fmt, fmt::format_args fmt_args, std::ostream *out)
{
    //assert(out != &std::cout);
    do_log(fmt, fmt_args, out ? out : &boost::nowide::cout);
}

void log(wstring_view fmt, fmt::wformat_args fmt_args, std::wostream *out)
{
    do_log(fmt, fmt_args, out ? out : &std::wcout);
}
} // namespace fd