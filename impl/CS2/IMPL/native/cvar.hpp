#pragma once

#include "functional/vfunc.h"

#include <cassert>

namespace fd::native::inline cs2
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

    auto first_iterator() -> iterator
    {
        iterator it;

        using fn_t = void (cvar_system::*)(iterator*);
        vfunc<fn_t>{12, this}(&it);
        return it;
    }

    iterator next_iterator(iterator it)
    {
        using fn_t = void (cvar_system::*)(iterator*, iterator);
        vfunc<fn_t>{13, this}(&it, it);
        return it;
    }

    convar* get(iterator index)
    {
        using fn_t = convar* (cvar_system::*)(iterator);
        return vfunc<fn_t>{37, this}(index);
    }

    convar* get(char const* name, size_t const length)
    {
        for (auto it = first_iterator(); it != static_cast<iterator>(-1); it = next_iterator(it))
        {
            auto const cvar = get(it);
            assert(cvar != nullptr);
            if (cvar->name[length] == '\0' && memcmp(cvar->name, name, length) == 0)
                return cvar;
        }

        return nullptr;
    }

    template <size_t N>
    convar* get(char const (&name)[N])
    {
        return get(name, N - 1);
    }
};
}