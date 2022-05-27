module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>

export module cheat.gui.render_interface;

using render_interface_base = Rml::RenderInterface;

struct custom_render_interface : render_interface_base
{
    void ReleaseTextures();
    virtual void RenderContext(Rml::Context* const ctx) = 0;
};

CHEAT_OBJECT(render_interface, custom_render_interface);
CHEAT_OBJECT(render_interface_raw, render_interface_base);

export namespace cheat::gui
{
    using ::render_interface;
    using ::render_interface_raw;
}
