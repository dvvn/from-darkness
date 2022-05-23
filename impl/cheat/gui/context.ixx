module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>

export module cheat.gui.context;

using _Ctx_ptr = Rml::Context*;

export namespace cheat::gui
{
    CHEAT_OBJECT(context, _Ctx_ptr);
}
