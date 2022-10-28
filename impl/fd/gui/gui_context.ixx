export module fd.gui.context;

export namespace fd::gui
{
    struct basic_context
    {
        virtual ~basic_context()               = default;
        virtual void release_textures()        = 0;
        virtual bool begin_frame()             = 0;
        virtual void end_frame() = 0;
    };

    basic_context* context;
} // namespace fd::gui