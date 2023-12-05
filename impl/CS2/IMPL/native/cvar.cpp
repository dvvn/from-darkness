#include "functional/vfunc.h"
#include "native/cvar.hpp"

#include <cassert>

namespace fd::native::inline cs2
{
auto cvar_system::first_iterator() -> iterator
{
    iterator it;

    using fn_t =
#ifdef _WIN32
        void (cvar_system::*)(iterator*);
#elif defined(__linux__)
        iterator (CCVar::*)();
#endif

#ifdef __linux__
    it =
#endif
        vfunc<fn_t>{12, this}(
#ifdef _WIN32
            &it
#endif
        );

    return it;
}

auto cvar_system::next_iterator(iterator it) -> iterator
{
    using fn_t =
#ifdef _WIN32
        void (cvar_system::*)(iterator*, iterator);
#elif defined(__linux__)
        CVarIterator_t (CCVar::*)(CVarIterator_t);
#endif

#ifdef __linux__
    it =
#endif
        vfunc<fn_t>{13, this}(
#ifdef _WIN32
            &it,
#endif
            it //
        );

    return it;
}

convar* cvar_system::get(iterator index)
{
    using fn_t = convar* (cvar_system::*)(iterator);

    return vfunc<fn_t>{37, this}(index);
}

convar* cvar_system::get(char const* name, size_t const length)
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
}