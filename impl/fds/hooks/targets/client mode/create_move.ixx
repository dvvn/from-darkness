module;

#include <string_view>

export module fds.hooks.client_mode.create_move;
export import fds.hooks.base;

export namespace fds::hooks::client_mode
{
    struct create_move : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::client_mode
