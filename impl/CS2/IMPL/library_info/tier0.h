#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/cvar.h"

namespace fd
{
template <>
class named_library_info<"tier0"_cs> final : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::cvar_system* cvar_system() const
        {
            return basic_interface_getter::get("VEngineCvar");
        }

        template <size_t I>
        native::cvar_system* get() const requires(I == 0u)
        {
            return cvar_system();
        }
    };

  public:
    named_library_info()
        : native_library_info{L"tier0.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
} // namespace fd