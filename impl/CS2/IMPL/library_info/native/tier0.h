#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/cvar.h"

namespace fd
{
using tier0_dll = named_library_info<"tier0">;

namespace detail
{
template <>
class library_object_getter<tier0_dll>
{
    native_library_interface_getter ifc_;

  public:
    library_object_getter(library_info const* linfo)
        : ifc_{linfo}
    {
    }

    native::cvar_system* cvar_system() const
    {
        return ifc_.find("VEngineCvar");
    }
};

template <size_t I>
auto get(library_object_getter<tier0_dll> const& getter)
{
    if constexpr (I == 0)
        return getter.cvar_system();
}
} // namespace detail
} // namespace fd
