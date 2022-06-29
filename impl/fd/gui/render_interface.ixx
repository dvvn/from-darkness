module;

#include <fd/object.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>

export module fd.gui.render_interface;

struct custom_render_interface : Rml::RenderInterface
{
    void ReleaseTextures();
    virtual void RenderContext(Rml::Context* const ctx) = 0;
};

FD_OBJECT(render_interface, custom_render_interface);
FD_OBJECT(render_interface_raw, Rml::RenderInterface);

export namespace fd::gui
{
    using ::render_interface;
    using ::render_interface_raw;
} // namespace fd::gui
