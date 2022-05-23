module;

#include <cheat/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module cheat.gui.system_interface;

using _Sys_ifc = Rml::SystemInterface;

export namespace cheat::gui
{
    CHEAT_OBJECT(system_interface, _Sys_ifc);
}
