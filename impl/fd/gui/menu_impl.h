#pragma once

#include <fd/gui/menu.h>
#include <fd/string.h>
#include <fd/tuple.h>

#define FD_GUI_RANDOM_TAB_BAR_NAME

namespace fd::gui
{
class tab_base
{
    string_view name_;

  public:
    virtual ~tab_base() = default;
    tab_base(string_view name);

  protected:
    bool new_frame();
    void end_frame();
};

template <typename Callback>
class tab final : public tab_base
{
    Callback callback_;

  public:
    tab(const string_view name, Callback callback)
        : tab_base(name)
        , callback_(std::move(callback))
    {
    }

    bool render()
    {
        const auto ret = new_frame();
        if (ret)
        {
            callback_();
            end_frame();
        }
        return ret;
    }
};

class tab_bar_base
{
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    string
#else
    string_view
#endif
        name_;

  public:
    tab_bar_base(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
        string_view name
#endif
    );

  protected:
    bool new_frame();
    void end_frame();
};

template <class... Tabs>
class tab_bar : public tab_bar_base
{
    tuple<Tabs...> tabs_;

  public:
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    tab_bar(Tabs... tabs)
        : tabs_(std::move(tabs)...)
#else
    tab_bar(string_view name, Tabs... tabs)
        : tab_bar_base(std::move(name))
        , tabs_(std::move(tabs)...)
#endif
    {
        static_assert((std::derived_from<Tabs, tab_base> && ...));
    }

    bool render()
    {
        const auto ret = new_frame();
        if (ret)
        {
            iterate(tabs_, [](auto& tab) {
                tab.render();
            });
            end_frame();
        }
        return ret;
    }
};

class menu_impl_base : public basic_menu
{
    bool visible_;
    bool visibleNext_;

  public:
#ifdef FD_HAVE_HOTKEY
    static constexpr struct
    {
        hotkey_source unload = this + __LINE__;
        hotkey_source toggle = this + __LINE__;
    } hotkeys;
#endif

    menu_impl_base();

    bool visible() const override;

    void show() override;
    void hide() override;
    void toggle() override;

  protected:
    bool new_frame(bool& visible);
    void end_frame(bool visible);
};

template <class... Tbar>
class menu_impl final : public menu_impl_base
{
    tuple<Tbar...> tabBars_;

  public:
    menu_impl(Tbar... bars)
        : tabBars_(std::move(bars)...)
    {
        static_assert((std::derived_from<Tbar, tab_bar_base> && ...));
    }

    bool render() override
    {
        auto visible = true;
        if (!new_frame(visible))
            return false;

        iterate(tabBars_, [](auto& bar) {
            bar.render();
        });

        end_frame(visible);
        return visible;
    }
};
} // namespace fd::gui