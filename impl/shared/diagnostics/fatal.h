#pragma once

#include <cassert>
#include <cstdlib>
#include <type_traits>

namespace fd
{
#ifdef _DEBUG
[[noreturn]]
inline void unreachable()
{
    assert(0);
}
#else
#ifdef __GNUC__
[[noreturn]]
inline __attribute__((always_inline)) void unreachable()
{
    __builtin_unreachable();
}
#elif defined(_MSC_VER)
[[noreturn]]
__forceinline void unreachable() // NOLINT(clang-diagnostic-language-extension-token)
{
    __assume(false);
}
#else
constexpr auto unreachable = std::abort;
#endif
#endif

template <typename = void, uint8_t N>
[[noreturn]]
constexpr void unreachable_ct()
{
    static_assert(N == N + 1);
}
} // namespace fd