#pragma once
#include "library_info/interface.h"
#include "library_info/interface_name.h"
#include "library_info/root_interface.h"
#include "native/engine_client.h"
#include "native/game_resource_service.h"

namespace fd
{
template <>
struct native::interface_name<native::engine_client>
{
    static constexpr auto& value = "Source2EngineToClient";
};

template <>
struct native::interface_name<native::game_resource_service>
{
    static constexpr auto& value = "GameResourceServiceClient";
};

class engine_lib final : public native_library_info
{
    class interface_getter : public basic_interface_getter
    {
        struct known_interfaces
        {
            native::engine_client* engine;
            native::game_resource_service* game_resource_service;
        };

      public:
        native::engine_client* engine() const
        {
            return find<native::engine_client>();
        }

        native::game_resource_service* game_resource_service() const
        {
            return find<native::game_resource_service>();
        }

        known_interfaces known() const
        {
            return find<native::engine_client, native::game_resource_service>();
        }
    };

  public:
    engine_lib()
        : native_library_info{L"engine2.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};

} // namespace fd