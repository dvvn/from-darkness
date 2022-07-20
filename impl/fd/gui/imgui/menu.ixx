module;

#include <fd/object.h>

#include <memory>

export module fd.gui.menu;
export import fd.gui.objects;
import fd.callback;

using namespace fd;
namespace obj = gui::objects;

struct basic_menu : virtual abstract_callback_custom<std::unique_ptr<obj::basic_window>>, virtual obj::basic_window
{
};

FD_OBJECT(menu, basic_menu);

export namespace fd
{
    namespace gui
    {
        using ::menu;
    }

    using ::fd::invoke; // from callback, object

} // namespace fd
