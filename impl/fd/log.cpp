#include "log.h"

#include <boost/container/small_vector.hpp>
#include <boost/nowide/iostream.hpp>
//
#include <fmt/chrono.h>

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>

using fmt::format_to;
using fmt::vformat_to;

namespace fd
{
[[maybe_unused]] //
static auto dummy = [] {
#ifdef _MSC_VER
    //constexpr char cp_utf16le[] = ".1200";
    //setlocale(LC_ALL, cp_utf16le);
    _setmode(_fileno(stdout), _O_WTEXT);
#else

#endif
    return std::ios::sync_with_stdio(false);
}();

template <typename T>
static void do_log(fmt::basic_string_view<T> fmt, auto fmt_args, std::basic_ostream<T> *out)
{
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

void log(fmt::string_view fmt, fmt::format_args fmt_args, std::ostream *out)
{
    if (!out)
        out = &boost::nowide::cout;
    else
    {
#ifdef _MSC_VER
        assert(out != &std::cout);
#endif
    }
    do_log(fmt, fmt_args, out);
}

void log(fmt::wstring_view fmt, fmt::wformat_args fmt_args, std::wostream *out)
{
    if (!out)
        out = &std::wcout;
    do_log(fmt, fmt_args, out);
}
}