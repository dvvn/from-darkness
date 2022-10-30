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
        string_view name_;
        std::vector<function_view<void() const>> data_;

      public:
        tab(const string_view name);

        bool render() const;
        void render_data() const;

        template <typename Fn>
        void store(Fn&& fn)
        {
            data_.emplace_back(std::forward<Fn>(fn));
        }
    };

    class tab_bar
    {
        string_view name_;
        std::vector<const tab*> data_;

      public:
        tab_bar(const string_view name);

        void render() const;
        void store(const tab& new_tab);
    };

    class menu_impl : public basic_menu
    {
        bool visible_;
        bool next_visible_;

        std::vector<const tab_bar*> data_;

      public:
        menu_impl();

        bool visible() const override;

        void show() override;
        void hide() override;
        void toggle() override;

        void render() override;

        void store(const tab_bar& new_tab_bar);
    };
} // namespace fd::gui