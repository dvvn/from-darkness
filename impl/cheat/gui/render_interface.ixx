module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>

export module cheat.gui.render_interface;

using _Ctx_ptr = Rml::Context*;
using _Render_ifc = Rml::RenderInterface;

struct custom_render_interface : _Render_ifc
{
    virtual void Init(void* const renderer = nullptr) = 0;
    void ReleaseTextures();
    virtual void RenderContext(_Ctx_ptr const ctx) = 0;
};

export namespace cheat::gui
{
    CHEAT_OBJECT(render_interface, custom_render_interface);
}
