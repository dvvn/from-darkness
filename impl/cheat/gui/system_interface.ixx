module;

#include <cheat/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module cheat.gui.system_interface;

using _Sys_ifc = Rml::SystemInterface;
constexpr size_t _Sys_idx = 0;

export namespace cheat::gui
{
    CHEAT_OBJECT(system_interface, _Sys_ifc, _Sys_idx);
}
