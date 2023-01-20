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

#ifdef FD_HAVE_HOTKEY
    enum hotkey_mode
    {
        press,
        held,
    };

    using hotkey_source = void*;

    enum hotkey_access
    {
        foreground = 1 << 0, // when any window visible
        background = 1 << 1, // when all windows invisible
        any = foreground | background
    };

    struct basic_hotkey
    {
        virtual ~basic_hotkey() = default;

        virtual string_view name() const = 0;
        virtual void        callback() const = 0;
    };
#endif
    struct basic_context
    {
        virtual ~basic_context() = default;

        virtual void                release_textures() = 0;
        virtual void                render(void* data) = 0;
        virtual process_keys_result process_keys(void* data) = 0;
#ifdef FD_HAVE_HOTKEY
        virtual basic_hotkey* find_hotkey(hotkey_source source) = 0;
#endif
    };
} // namespace fd::gui