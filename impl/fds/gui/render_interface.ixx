module;

#include <fds/core/object.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>

export module fds.gui.render_interface;

struct custom_render_interface : Rml::RenderInterface
{
    void ReleaseTextures();
    virtual void RenderContext(Rml::Context* const ctx) = 0;
};

FDS_OBJECT(render_interface, custom_render_interface);
FDS_OBJECT(render_interface_raw, Rml::RenderInterface);

export namespace fds::gui
{
    using ::render_interface;
    using ::render_interface_raw;
} // namespace fds::gui
