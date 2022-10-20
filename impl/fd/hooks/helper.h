#pragma once

#include <tuple>

namespace fd::hooks
{
    template <typename... H>
    class holder
    {
        std::tuple<H...> hooks_;

      public:
        holder(H&&... hooks)
            : hooks_(std::move(hooks)...)
        {
        }

        bool enable()
        {
            return (std::get<H>(hooks_).enable() && ...);
        }

        bool disable()
        {
            return (std::get<H>(hooks_).disable() && ...);
        }
    };
} // namespace fd::hooks