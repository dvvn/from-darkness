#pragma once

#include <fd/gui/basic_tab.h>

namespace fd
{
template <typename Callback>
class tab final : public basic_tab, public renderable
{
    Callback callback_;

  public:
    tab(std::string_view name, Callback callback)
        : basic_tab(name)
        , callback_(std::move(callback))
    {
    }

    bool render() override
    {
        auto ret = begin_frame();
        if (ret)
        {
            callback_();
            end_frame();
        }
        return ret;
    }
};
}