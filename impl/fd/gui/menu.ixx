module;

export module fd.gui.menu;

export namespace fd::gui
{
    struct basic_menu
    {
        virtual ~basic_menu();

        virtual bool visible() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual void toggle();

        virtual void render() = 0;
    };

    basic_menu* menu;
} // namespace fd::gui