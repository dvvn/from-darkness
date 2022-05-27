module;

#include <cheat/core/object.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>

export module cheat.gui.render_interface;

struct custom_render_interface : Rml::RenderInterface
{
    void ReleaseTextures();
    virtual void RenderContext(Rml::Context* const ctx) = 0;
};

CHEAT_OBJECT(render_interface, custom_render_interface);
CHEAT_OBJECT(render_interface_raw, Rml::RenderInterface);

export namespace cheat::gui
{
    using ::render_interface;
    using ::render_interface_raw;
}
