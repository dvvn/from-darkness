module;

#include <vector>

export module fd.gui.menu.impl;
export import fd.gui.menu;
export import fd.string;
import fd.functional.fn;

export namespace fd::gui
{

    class tab
    {
        using callback_type = function_view<void() const>;

        string_view name_;
        std::vector<callback_type> callbacks_;

      public:
        tab(const string_view name);

        bool render() const;
        void render_data() const;

        void store(callback_type callback);
    };

    class tab_bar
    {
        string_view name_;
        std::vector<tab*> tabs_;

      public:
        tab_bar(const string_view name);

        void render() const;

        void store(tab& new_tab);
    };

    class menu_impl : public basic_menu
    {
        bool visible_;
        bool next_visible_;

        std::vector<tab_bar*> tab_bars_;

      public:
        menu_impl();

        bool visible() const override;

        void show() override;
        void hide() override;
        void toggle() override;

        bool render() override;

        void store(tab_bar& new_tab_bar);
    };
} // namespace fd::gui