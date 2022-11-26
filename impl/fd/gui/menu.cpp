#include <fd/gui/menu.h>

namespace fd::gui
{
    basic_menu::~basic_menu() = default;

    void basic_menu::toggle()
    {
        if (visible())
            hide();
        else
            show();
    }

    basic_menu* menu;
} // namespace fd::gui