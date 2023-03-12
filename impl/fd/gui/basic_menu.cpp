#include <fd/gui/basic_menu.h>

namespace fd
{

void basic_menu::toggle()
{
    if (visible())
        close();
    else
        show();
}
} // namespace fd