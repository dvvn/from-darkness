module;

export module fd.gui.menu.impl;
export import fd.gui.menu;

export namespace fd::gui
{
    class menu_impl : public basic_menu
    {
        bool visible_;
        bool next_visible_;

      public:
        menu_impl();

        bool visible() const override;

        void show() override;
        void hide() override;
        void toggle() override;

        void render() override;
    };
} // namespace fd::gui