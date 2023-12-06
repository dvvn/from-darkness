#pragma once
#include "functional/cast.h"
#include "library_info/impl/interface.h"
#include "library_info/impl/root_interface.h"
#include "native/engine_client.hpp"

namespace fd
{
class engine_library_info : public native_library_info
{
    struct interface_getter : basic_interface_getter
    {
        native::engine_client* engine() const
        {
            return safe_cast_from(linfo_->find("Source2EngineToClient"_ifc));
        }
    };

  public:
    engine_library_info()
        : native_library_info{L"engine2"_dll}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
}