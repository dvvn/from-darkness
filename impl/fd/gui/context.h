#pragma once

#include <fd/string.h>

namespace fd::gui
{
    enum process_keys_result
    {
        instant,
        native,
        def
    };

    enum hotkey_mode
    {
        press,
        held,
    };

    using hotkey_source = uint8_t;

    enum hotkey_access
    {
        foreground = 1 << 0, // when any window visible
        background = 1 << 1, // when all windows invisible
        any        = foreground | background
    };

    struct basic_hotkey
    {
        virtual ~basic_hotkey() = default;

        virtual hotkey_source source() const = 0;
        virtual hotkey_mode mode() const     = 0;
        virtual hotkey_access access() const = 0;
        virtual string_view name() const     = 0;
        virtual void callback() const        = 0;
    };

    struct basic_context
    {
        virtual ~basic_context() = default;

        virtual void release_textures()                      = 0;
        virtual void render(void* data)                      = 0;
        virtual process_keys_result process_keys(void* data) = 0;

        virtual basic_hotkey* find_basic_hotkey(hotkey_source source, hotkey_mode mode) = 0;
    };
} // namespace fd::gui