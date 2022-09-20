#pragma once

#include <tuple>

namespace fd
{
    template <typename... H>
    bool load_hooks(std::tuple<H...>& hooks)
    {
        return (std::get<H>(hooks).enable() && ...);
    }

    template <typename... H>
    bool disable_hooks(std::tuple<H...>& hooks)
    {
        return (std::get<H>(hooks).disable() && ...);
    }
} // namespace fd