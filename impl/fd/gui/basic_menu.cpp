#include <fd/gui/basic_menu.h>

namespace fd
{
basic_menu::~basic_menu() = default;

void basic_menu::toggle()
{
    if (visible())
        hide();
    else
        show();
}
} // namespace fd