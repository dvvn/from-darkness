module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>

export module cheat.gui.context;

using _Ctx_ptr = Rml::Context*;

constexpr size_t _Ctx_idx = 0;

export namespace cheat::gui
{
    CHEAT_OBJECT(context, _Ctx_ptr, _Ctx_idx);
}
