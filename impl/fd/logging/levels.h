#pragma once

#include <type_traits>

namespace fd
{
enum class log_level : uint8_t
{
    off,
    info     = 1 << 0,
    warn     = 1 << 1,
    err      = 1 << 2,
    critical = 1 << 3,
    debug    = info | warn | err | critical,
    trace    = 1 << 4
};

constexpr bool operator&(log_level lhs, log_level rhs)
{
    using type = std::underlying_type_t<log_level>;
    return static_cast<type>(lhs) & static_cast<type>(rhs);
}
}