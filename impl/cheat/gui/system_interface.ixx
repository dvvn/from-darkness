module;

#include <cheat/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module cheat.gui.system_interface;

CHEAT_OBJECT(system_interface, Rml::SystemInterface);

export namespace cheat::gui
{
    using ::system_interface;
}
