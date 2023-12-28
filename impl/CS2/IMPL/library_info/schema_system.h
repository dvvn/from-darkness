#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/schema_system.h"

namespace fd
{
template <>
class named_library_info<"schemasystem"_cs> final : native_library_info
{
    struct interface_getter final : basic_interface_getter
    {
        native::schema_system* schema_system() const
        {
            return basic_interface_getter::get("SchemaSystem");
        }

        template <size_t I>
        native::schema_system* get() const requires(I == 0u)
        {
            return schema_system();
        }
    };

  public:
    named_library_info()
        : native_library_info{L"schemasystem.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
} // namespace fd