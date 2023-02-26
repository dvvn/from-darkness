#pragma once

#include <fd/gui/menu_base.h>

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

namespace fd
{
template <typename Callback>
class tab final : public tab_base
{
    Callback callback_;

  public:
    tab(std::string_view name, Callback callback)
        : tab_base(name)
        , callback_(std::move(callback))
    {
    }

    bool render()
    {
        auto ret = new_frame();
        if (ret)
        {
            callback_();
            end_frame();
        }
        return ret;
    }
};

template <class... Tabs>
class tab_bar final : public tab_bar_base
{
    boost::hana::tuple<Tabs...> tabs_;

  public:
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    tab_bar(Tabs... tabs)
        : tabs_(std::move(tabs)...)
#else
    tab_bar(std::string_view name, Tabs... tabs)
        : tab_bar_base(std::move(name))
        , tabs_(std::move(tabs)...)
#endif
    {
        static_assert((std::derived_from<Tabs, tab_base> && ...));
    }

    bool render()
    {
        auto ret = new_frame();
        if (ret)
        {
            boost::hana::for_each(
                tabs_,
                [](auto& t)
                {
                    t.render();
                }
            );
            end_frame();
        }
        return ret;
    }
};

template <class... Tbar>
class menu final : public menu_base
{
    boost::hana::tuple<Tbar...> tabBars_;

  public:
    menu(Tbar... bars)
        : tabBars_(std::move(bars)...)
    {
        static_assert((std::derived_from<Tbar, tab_bar_base> && ...));
    }

    bool render() override
    {
        auto visible = true;
        if (!new_frame(visible))
            return false;

        boost::hana::for_each(
            tabBars_,
            [](auto& tb)
            {
                tb.render();
            }
        );

        end_frame(visible);
        return visible;
    }
};
}