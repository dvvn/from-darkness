#pragma once

#include <fd/functional.h>
#include <fd/gui/menu.h>
#include <fd/string.h>

#include <vector>

namespace fd::gui
{
    class tab
    {
        using callback_type = function<void() const>;

        string_view                name_;
        std::vector<callback_type> callbacks_;

      public:
        tab(string_view name);

        bool render() const;
        void render_data() const;

        void store(callback_type&& callback);
    };

    class tab_bar
    {
        string_view       name_;
        std::vector<tab*> tabs_;

      public:
        tab_bar(string_view name);

        void render() const;

        void store(tab& newTab);
    };

    class menu final : public basic_menu
    {
        bool visible_;
        bool visibleNext_;

        std::vector<tab_bar*> tabBars_;

      public:
#ifdef FD_HAVE_HOTKEY
        static constexpr struct
        {
            hotkey_source unload = this + __LINE__;
            hotkey_source toggle = this + __LINE__;
        } hotkeys;
#endif

        menu();

        bool visible() const override;

        void show() override;
        void hide() override;
        void toggle() override;

        bool render() override;

        void store(tab_bar& newTabBar);
    };
} // namespace fd::gui