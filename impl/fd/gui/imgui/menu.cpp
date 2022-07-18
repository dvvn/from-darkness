module;

#include <fd/object.h>

#include <functional>

module fd.gui.menu;
import fd.gui.widgets;

struct menu_impl final : basic_menu
{
    virtual void operator()() override
    {
        using namespace fd::gui;

        static bool b = false;

        window("test", [] {
            check_box("cbx", b);
        });
    }
};

FD_OBJECT_BIND_TYPE(menu, menu_impl);
