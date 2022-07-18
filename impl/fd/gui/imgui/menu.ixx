module;

#include <fd/object.h>

export module fd.gui.menu;

struct basic_menu
{
    virtual ~basic_menu() = default;

    virtual void operator()() = 0;
};

FD_OBJECT(menu, basic_menu);

export namespace fd::gui
{
    using ::menu;
}
