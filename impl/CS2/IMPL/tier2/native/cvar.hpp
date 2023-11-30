#pragma once

#include "tier2/core.h"

#include <cstdint>

namespace FD_TIER2(native, cs2)
{
class convar
{
  public:
    char const* name;

    /*template <typename T>
    T GetValue()
    {
        return CPointer(this).GetField<T>(0x40);
    }*/
};

// VEngineCvar007
class cvar_system
{
  public:
    using iterator = uint64_t;

    iterator first_iterator();
    iterator next_iterator(iterator it);

    convar* get(iterator index);
    convar* get(char const* name, size_t length);

    template <size_t N>
    convar* get(char const (&name)[N])
    {
        return get(name, N - 1);
    }
};
}