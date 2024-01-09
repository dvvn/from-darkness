#include "string/static.h"
#include "test_holder.h"

namespace fd
{
FD_ADD_TEST([] {
    assert("hello"_cs == "hello");
    assert("hello" == "hello"_cs);
    assert("hello"_ss == "hello");
    assert("hello" == "hello"_ss);
    assert("hello"_cs == "hello"_ss);
    assert(L"hello" == "hello"_cs);
    assert(L"hello"_ss == "hello");

    assert("part1" + "part2"_cs == "part1part2");
    assert(static_string<10>("1") + "2" + L"3"_cs + "4"_ss == "1234");
});
} // namespace fd
