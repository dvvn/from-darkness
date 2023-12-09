#pragma once
#include "functional/cast.h"
#include "native/cvar.h"
#include "library_info/interface.h"
#include "library_info/root_interface.h"

namespace fd
{
class tier0_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::cvar_system* cvar_system() const
        {
            return safe_cast_from(find("VEngineCvar"));
        }
    };

  public:
    tier0_library_info()
        : native_library_info{L"tier0.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
} // namespace fd