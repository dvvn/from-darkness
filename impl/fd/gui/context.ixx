module;

#include <fd/core/object.h>

#include <RmlUi/Core/Context.h>

export module fd.gui.context;

FD_OBJECT(context, Rml::Context*);

export namespace fd::gui
{
    using ::context;
}
