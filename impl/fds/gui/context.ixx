module;

#include <fds/core/object.h>

#include <RmlUi/Core/Context.h>

export module fds.gui.context;

FDS_OBJECT(context, Rml::Context*);

export namespace fds::gui
{
    using ::context;
}
