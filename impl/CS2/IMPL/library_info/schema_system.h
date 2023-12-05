#pragma once
#include "functional/cast.h"
#include "library_info/interface.h"
#include "library_info/root_interface.h"
#include "native/schema_system.hpp"

namespace fd
{
class schema_system_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::schema_system* schema_system() const
        {
            return safe_cast_from(linfo_->find("SchemaSystem"_ifc));
        }
    };

  public:
    schema_system_library_info()
        : native_library_info{L"schemasystem"_dll}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
}