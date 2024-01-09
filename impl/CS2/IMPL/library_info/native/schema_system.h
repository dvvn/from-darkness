#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/schema_system.h"

namespace fd
{
using schema_system_dll=named_library_info<"schemasystem">;

namespace detail
{
template <>
class library_object_getter<schema_system_dll>
{
    native_library_interface_getter ifc_;

  public:
    library_object_getter(library_info const* linfo)
        : ifc_{linfo}
    {
    }

    native::schema_system* schema_system() const
    {
        return ifc_.find("SchemaSystem");
    }
};

template <size_t I>
auto get(library_object_getter<schema_system_dll> const& getter)
{
    if constexpr (I == 0)
        return getter.schema_system();
}
} // namespace detail
} // namespace fd