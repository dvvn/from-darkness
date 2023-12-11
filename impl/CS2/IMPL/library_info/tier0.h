#pragma once
#include "library_info/interface.h"
#include "library_info/root_interface.h"
#include "native/cvar.h"

namespace fd
{
template <>
struct native::interface_name<native::cvar_system>
{
    static constexpr auto& value = "VEngineCvar";
};

class tier0_library_info : public native_library_info
{
    class interface_getter : public basic_interface_getter
    {
        struct known_interfaces
        {
            native::cvar_system* cvar_system;
        };

      public:
        native::cvar_system* cvar_system() const
        {
            return find<native::cvar_system>();
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