#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/engine_client.h"
#include "native/game_resource_service.h"

namespace fd
{
using engine2_dll = named_library_info<"engine2">;

namespace detail
{
template <>
class library_object_getter<engine2_dll>
{
    native_library_interface_getter ifc_;

  public:
    library_object_getter(library_info const* linfo)
        : ifc_{linfo}
    {
    }

    native::engine_client* engine() const
    {
        return ifc_.find("Source2EngineToClient");
    }

    native::game_resource_service* game_resource_service() const
    {
        return ifc_.find("GameResourceServiceClient");
    }
};

template <size_t I>
auto get(library_object_getter<engine2_dll> const& getter)
{
    if constexpr (I == 0)
        return getter.engine();
    else if constexpr (I == 1)
        return getter.game_resource_service();
}
} // namespace detail
} // namespace fd
