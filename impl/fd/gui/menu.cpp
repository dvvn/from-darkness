module;

module fd.gui.menu;

using namespace fd::gui;

basic_menu::~basic_menu() = default;

void basic_menu::toggle()
{
    if (visible())
        hide();
    else
        show();
}