#pragma once
#include "library_info/interface.h"
#include "library_info/root_interface.h"
#include "native/schema_system.h"

namespace fd
{
template <>
struct native::interface_name<native::schema_system>
{
    static constexpr auto& value = "SchemaSystem";
};

class schema_system_library_info : public native_library_info
{
    class interface_getter : public basic_interface_getter
    {
        struct known_interfaces
        {
            native::schema_system* schema_system;
        };

      public:
        native::schema_system* schema_system() const
        {
            return find<native::schema_system>();
        }
    };

  public:
    schema_system_library_info()
        : native_library_info{L"schemasystem.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
} // namespace fd