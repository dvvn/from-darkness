module;

#include <fd/object.h>

export module fd.gui.menu;
export import fd.gui.objects;
export import fd.callback;
import fd.memory;

using namespace fd;
namespace obj = gui::objects;

struct basic_menu : virtual abstract_callback_custom<unique_ptr<obj::basic_window>>, virtual obj::basic_window
{
};

FD_OBJECT(menu, basic_menu);

export namespace fd::gui
{
    using ::menu;
} // namespace fd::gui
