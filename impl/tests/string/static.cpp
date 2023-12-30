#include "string/static.h"

namespace fd
{
inline namespace literals
{
template <basic_static_string Str>
constexpr auto operator"" _ss() -> std::remove_const_t<decltype(Str)>
{
    return Str;
}
} // namespace literals

static_assert("hello"_cs == "hello");
static_assert("hello" == "hello"_cs);
static_assert("hello"_ss == "hello");
static_assert("hello" == "hello"_ss);
static_assert("hello"_cs == "hello"_ss);
static_assert(L"hello" == "hello"_cs);
static_assert(L"hello"_ss == "hello");

static_assert("part1" + "part2"_cs == "part1part2");
static_assert(static_string<10>("1") + "2" + L"3"_cs + "4"_ss == "1234");
} // namespace fd
