#include <fd/assert_handler.h>

using namespace fd;

static basic_assert_handler* _AssertHandler = nullptr;

void basic_assert_handler::set(basic_assert_handler* handler)
{
    _AssertHandler = handler;
}

namespace fd
{
    void run_assert(const assert_data& data, const char*)
    {
        _AssertHandler->run(data);
    }

    void run_assert(const assert_data& data, const bool exprResult)
    {
        if (!exprResult)
            _AssertHandler->run(data);
    }

    void run_panic_assert(const assert_data& data)
    {
        _AssertHandler->run_panic(data);
    }
}