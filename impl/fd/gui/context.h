#pragma once

namespace fd::gui
{
    enum process_keys_result
    {
        instant,
        native,
        def
    };

    struct basic_context
    {
        virtual ~basic_context() = default;

        virtual void release_textures()                      = 0;
        virtual void render(void* data)                      = 0;
        virtual process_keys_result process_keys(void* data) = 0;
    };
} // namespace fd::gui