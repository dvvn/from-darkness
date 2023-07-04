#pragma once

#include <cstdint>

namespace fd
{
struct basic_xref
{
    using pointer    = uintptr_t const *;
    using value_type = uintptr_t;

    virtual pointer get() const = 0;
};
} // namespace fd