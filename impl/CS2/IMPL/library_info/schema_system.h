﻿#pragma once
#include "functional/cast.h"
#include "library_info/impl/interface.h"
#include "library_info/impl/root_interface.h"
#include "native/schema_system.hpp"

#undef interface

namespace fd
{
class schema_system_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::schema_system* schema_system() const
        {
            return safe_cast_from(find("SchemaSystem"_ifc));
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