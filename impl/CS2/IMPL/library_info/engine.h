#pragma once
#include "library_info/interface_getter.h"
#include "library_info/literals.h"
#include "native/engine_client.h"
#include "native/game_resource_service.h"

namespace fd
{
template <>
class named_library_info<"engine2"_cs> final : public native_library_info
{
    struct interface_getter final : basic_interface_getter
    {
        native::engine_client* engine() const
        {
            return basic_interface_getter::get("Source2EngineToClient");
        }

        native::game_resource_service* game_resource_service() const
        {
            return basic_interface_getter::get("GameResourceServiceClient");
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

  public:
    named_library_info()
        : native_library_info{L"engine2.dll"}
    {
    }

    interface_getter interface() const
    {
        return {this};
    }
};
} // namespace fd
