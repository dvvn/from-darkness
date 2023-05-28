#include "log.h"

#include <boost/container/small_vector.hpp>
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

namespace fd
{
#ifdef _DEBUG
static bool logging_initialized = false;
#endif

bool init_logging()
{
    assert(!logging_initialized);
#ifdef _WIN32
    // constexpr char cp_utf16le[] = ".1200";
    // setlocale(LC_ALL, cp_utf16le);
    if (_setmode(_fileno(stdout), _O_WTEXT) == -1)
        return false;
#else

#endif
    std::ios::sync_with_stdio(false);
#ifdef _DEBUG
    logging_initialized = true;
#endif
    return true;
}

template <typename T>
static void do_log(basic_string_view<T> fmt, auto &fmt_args, std::basic_ostream<T> *out)
{
    assert(logging_initialized);

    auto time = std::chrono::system_clock::now().time_since_epoch();

    boost::container::small_vector<T, 512> buff;
    auto it = std::back_inserter(buff);

    format_to(it, "[{:%H:%M:%S}]", time);
    buff.push_back(' ');
    vformat_to(it, fmt, fmt_args);
    buff.push_back('\n');

    out->write(buff.data(), buff.size());
    out->flush();
}

void log(string_view fmt, fmt::format_args fmt_args, std::ostream *out)
{
    assert(out != &std::cout);
    do_log(fmt, fmt_args, out ? out : &boost::nowide::cout);
}

void log(wstring_view fmt, fmt::wformat_args fmt_args, std::wostream *out)
{
    do_log(fmt, fmt_args, out ? out : &std::wcout);
}
} // namespace fd