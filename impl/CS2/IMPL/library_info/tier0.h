#pragma once
#include "library_info/root_interface.h"
#include "native/cvar.h"

namespace fd
{
template <>
struct detail::library_interface_getter<struct tier0_lib> : library_interface_getter<>
{
    using library_interface_getter<>::get;

    native::cvar_system* cvar_system() const
    {
        return get("VEngineCvar");
    }

    template <size_t I>
    native::cvar_system* get() const requires(I == 0u)
    {
        return cvar_system();
    }
};

struct tier0_lib : native_library_info
{
    tier0_lib()
        : native_library_info{L"tier0.dll"}
    {
    }

    detail::library_interface_getter<tier0_lib> interface() const
    {
        return {this};
    }
};
} // namespace fd