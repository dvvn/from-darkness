#pragma once
#include "library_info/root_interface.h"
#include "native/schema_system.h"

namespace fd
{
template <>
struct detail::library_interface_getter<struct schema_system_lib> : library_interface_getter<>
{
    using library_interface_getter<>::get;

    native::schema_system* schema_system() const
    {
        return get("SchemaSystem");
    }

    template <size_t I>
    native::schema_system* get() const requires(I == 0u)
    {
        return schema_system();
    }
};

struct schema_system_lib : native_library_info
{
    schema_system_lib()
        : native_library_info{L"schemasystem.dll"}
    {
    }

    detail::library_interface_getter<schema_system_lib> interface() const
    {
        return {this};
    }
};
} // namespace fd