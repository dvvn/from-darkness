module;

#include <string_view>

export module fds.hooks.client.frame_stage_notify;
export import fds.hooks.base;

export namespace fds::hooks::client
{
    struct frame_stage_notify : class_base
    {
        std::string_view name() const final;
    };
} // namespace fds::hooks::client
