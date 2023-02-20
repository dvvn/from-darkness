#pragma once

#include <fd/hook.h>

namespace fd
{
class hook_impl : public basic_hook
{
    void*       entry_;
    string_view name_;

  public:
    hook_impl(string_view name);
    ~hook_impl() override;

    hook_impl(const hook_impl&) = delete;
    hook_impl(hook_impl&& other) noexcept;

    bool enable() override;
    bool disable() override;

    string_view name() const override;

    bool initialized() const override;
    bool active() const override;

    void* get_original_method() const;
    bool  init(void* target, void* replace);

    explicit operator bool() const;
};

} // namespace fd