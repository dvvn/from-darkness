#pragma once

#include <fd/gui/basic_tab_bar.h>
#include <fd/gui/tab.h>

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

namespace fd
{
template <class... Tabs>
class tab_bar final : public basic_tab_bar, public renderable
{
    boost::hana::tuple<Tabs...> tabs_;

  public:
#ifdef FD_GUI_RANDOM_TAB_BAR_NAME
    tab_bar(Tabs... tabs)
        : tabs_(std::move(tabs)...)
#else
    tab_bar(string_type name, Tabs... tabs)
        : tab_bar_base(std::move(name))
        , tabs_(std::move(tabs)...)
#endif
    {
    }

    bool render() override
    {
        auto ret = begin_frame();
        if (ret)
        {
            boost::hana::for_each(tabs_, []<typename Fn>(tab<Fn> &t) { t.render(); });
            end_frame();
        }
        return ret;
    }
};

}