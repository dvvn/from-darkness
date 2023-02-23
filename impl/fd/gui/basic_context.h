#pragma once

#include <cstdint>

namespace fd
{
struct basic_gui_context
{
    enum class keys_return : uint8_t
    {
        instant,
        native,
        def
    };

    virtual ~basic_gui_context() = default;

    virtual void        release_textures()       = 0;
    virtual void        render(void* data)       = 0;
    virtual keys_return process_keys(void* data) = 0;
};
} // namespace fd