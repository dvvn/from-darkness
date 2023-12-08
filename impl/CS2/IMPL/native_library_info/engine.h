#pragma once
#include "functional/cast.h"
#include "native/engine_client.h"
#include "native_library_info/impl/interface.h"
#include "native_library_info/impl/root_interface.h"

namespace fd
{
class engine_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::engine_client* engine() const
        {
            return safe_cast_from(find("Source2EngineToClient"));
        }
    };

  public:
    engine_library_info()
        : native_library_info{L"engine2.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
}