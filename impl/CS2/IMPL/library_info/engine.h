#pragma once
#include "library_info/interface.h"
#include "library_info/root_interface.h"
#include "native/engine_client.h"
#include "native/game_resource_service.h"

namespace fd
{
template <>
struct detail::library_interface_getter<struct engine_lib> : library_interface_getter<>
{
    using library_interface_getter<>::get;

    native::engine_client* engine() const
    {
        return get("Source2EngineToClient");
    }

    native::game_resource_service* game_resource_service() const
    {
        return get("GameResourceServiceClient");
    }

    template <size_t I>
    native::engine_client* get() const requires(I == 0u)
    {
        return engine();
    }

    template <size_t I>
    native::game_resource_service* get() const requires(I == 1u)
    {
        return game_resource_service();
    }
};

struct engine_lib final : native_library_info
{
    engine_lib()
        : native_library_info{L"engine2.dll"}
    {
    }

    detail::library_interface_getter<engine_lib> interface() const
    {
        return {this};
    }
};
} // namespace fd
