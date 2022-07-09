module;

#include <fd/object.h>

export module fd.gui.basic_render_interface;

export namespace fd::gui
{
    struct basic_render_interface
    {
        virtual ~basic_render_interface() = default;

        virtual bool skip_frame() const = 0;
        virtual void release_textures() = 0;
        virtual bool operator()()       = 0;
    };

    FD_OBJECT(render_interface, basic_render_interface);
} // namespace fd::gui
