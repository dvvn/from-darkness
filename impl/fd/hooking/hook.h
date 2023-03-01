#pragma once

#include <fd/hooking/basic_hook.h>

#include <string_view>

namespace fd
{
class hook : public basic_hook
{
    void*            entry_;
    std::string_view name_;

  public:
    ~hook() override;

    hook();
    hook(std::string_view name);

    hook(hook const&) = delete;
    hook(hook&& other) noexcept;
    hook& operator=(hook const& other) = delete;
    hook& operator=(hook&& other) noexcept;

    bool enable() final;
    bool disable() final;

    char const*      name() const final;
    std::string_view native_name() const;

    bool initialized() const final;
    bool active() const final;

    void* get_original_method() const;
    bool  init(void* target, void* replace);

    explicit operator bool() const;
};

} // namespace fd