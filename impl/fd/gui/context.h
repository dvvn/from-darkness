#pragma once

namespace fd::gui
{
    struct basic_context
    {
        virtual ~basic_context()              = default;
        virtual void release_textures()       = 0;
        virtual void render(void* data)       = 0;
        virtual char process_keys(void* data) = 0;
    };

    extern basic_context* context;
} // namespace fd::gui