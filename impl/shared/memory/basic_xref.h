#pragma once

#include <cstdint>

namespace fd
{
struct basic_xref
{
  protected:
    ~basic_xref() = default;

  public:
    using pointer    = uintptr_t const *;
    using value_type = uintptr_t;

    virtual pointer get() const = 0;
};
} // namespace fd