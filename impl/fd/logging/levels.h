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
    trace    = 1 << 4,
};

constexpr bool have_log_level(log_level level, log_level checked)
{
    using type = std::underlying_type_t<log_level>;
    auto l     = static_cast<type>(level);
    auto r     = static_cast<type>(checked);

    return l & r && l >= r;
}

//constexpr log_level &operator&=(log_level &lhs, log_level rhs)
//{
//    using type = std::underlying_type_t<log_level>;
//    return lhs = static_cast<log_level>(static_cast<type>(lhs) & static_cast<type>(rhs));
//}

constexpr log_level operator~(log_level val)
{
    using type = std::underlying_type_t<log_level>;
    return static_cast<log_level>(~static_cast<type>(val));
}

constexpr log_level operator|(log_level lhs, log_level rhs)
{
    using type = std::underlying_type_t<log_level>;
    return static_cast<log_level>(static_cast<type>(lhs) | static_cast<type>(rhs));
}
}