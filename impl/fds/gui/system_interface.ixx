module;

#include <fds/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

export module fds.gui.system_interface;

FDS_OBJECT(system_interface, Rml::SystemInterface);

export namespace fds::gui
{
    using ::system_interface;
}
