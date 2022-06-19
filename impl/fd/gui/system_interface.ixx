module;

#include <fd/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module fd.gui.system_interface;

FD_OBJECT(system_interface, Rml::SystemInterface);

export namespace fd::gui
{
    using ::system_interface;
}
