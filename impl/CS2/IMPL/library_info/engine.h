#pragma once
#include "functional/cast.h"
#include "library_info/interface.h"
#include "library_info/root_interface.h"
#include "native/engine_client.h"

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
} // namespace fd