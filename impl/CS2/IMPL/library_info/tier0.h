#pragma once
#include "functional/cast.h"
#include "library_info/impl/interface.h"
#include "library_info/impl/root_interface.h"
#include "native/cvar.hpp"

namespace fd
{
class tier0_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::cvar_system* cvar_system() const
        {
            return safe_cast_from(linfo_->find("VEngineCvar"_ifc));
        }
    };

  public:
    tier0_library_info()
        : native_library_info{L"tier0"_dll}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
}