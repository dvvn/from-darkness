#pragma once

#include "preprocessor/random.h"

#include <cassert>
#include <cstdint>

namespace fd
{
uint8_t add_test(void (*fn)()) noexcept;
void run_tests() noexcept;
} // namespace fd

#ifdef _DEBUG
#define FD_ADD_TEST(...) static uint8_t const FD_RANDOM_NAME = add_test(__VA_ARGS__);
#else
#define FD_ADD_TEST(...)
#endif