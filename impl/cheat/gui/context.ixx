module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>

export module cheat.gui.context;

CHEAT_OBJECT(context, Rml::Context*);

export namespace cheat::gui
{
    using ::context;
}
