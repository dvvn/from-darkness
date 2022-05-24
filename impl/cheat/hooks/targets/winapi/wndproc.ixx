module;

#include <string_view>

export module cheat.hooks.winapi.wndproc;
export import cheat.hooks.base;

export namespace cheat::hooks::winapi
{
    struct wndproc : static_base
    {
        std::string_view name() const noexcept final;
    };
} // namespace cheat::hooks::winapi
