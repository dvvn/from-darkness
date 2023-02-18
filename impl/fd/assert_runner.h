#pragma once

#include <fd/assert_data.h>

namespace fd
{
void run_assert(const assert_data& data, const char*);
void run_assert(const assert_data& data, bool exprResult);

[[noreturn]]
void run_panic_assert(const assert_data& data);
} // namespace fd