#pragma once

#include <cstdint>

namespace fd
{
enum class log_level : uint8_t
{
    off,
    info,
    warn,
    err,
    critical,
    debug,
    // trace
};
}