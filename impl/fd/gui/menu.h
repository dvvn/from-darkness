#pragma once
#include <fd/gui/context.h>

namespace fd::gui
{
    struct basic_menu
    {
        virtual ~basic_menu();

        virtual bool visible() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual void toggle();

        virtual bool render(basic_context* ctx) = 0;
    };
} // namespace fd::gui