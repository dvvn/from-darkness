#pragma once

#include <fd/gui/menu_base.h>
#include <fd/gui/tab_bar.h>

namespace fd
{
template <class... Tbar>
class menu final : public menu_base, public renderable
{
    boost::hana::tuple<Tbar...> tab_bars_;

  public:
    menu(Tbar... bars)
        : tab_bars_(std::move(bars)...)
    {
    }

    bool render() override
    {
        if (!begin_frame())
            return false;

        if (!collapsed())
            boost::hana::for_each(tab_bars_, []<class... T>(tab_bar<T...> &tb) { tb.render(); });

        end_frame();
        return true;
    }
};
} // namespace fd