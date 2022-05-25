module;

#include <cheat/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module cheat.gui.system_interface;

using system_interface_base = Rml::SystemInterface;
CHEAT_OBJECT(system_interface, system_interface_base);

export namespace cheat::gui
{
    using ::system_interface;
}
