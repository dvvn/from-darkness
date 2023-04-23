#pragma once

#include <fd/hooking/basic_hook.h>

#include <string_view>

namespace fd
{
class hook final : public basic_hook
{
    std::string name_;

#if __has_include(<subhook.h>)
    void *entry_ = nullptr;
#elif __has_include(<minhook.h>)
    void *target_     = nullptr;
    void *trampoline_ = nullptr;
    bool active_      = false;
#endif

    hook &operator=(hook const &other) = default;
    hook(hook const &)                 = default;

  public:
    ~hook() override;

    hook();
    hook(std::string const &name);
    hook(std::string &&name);

    hook(hook &&other) noexcept;

    hook &operator=(hook &&other) noexcept;

    bool enable() override;
    bool disable() override;

    char const *name() const override;
    std::string_view native_name() const;

    bool initialized() const override;
    bool active() const override;

    void *get_original_method() const;
    bool init(void *target, void *replace);

    explicit operator bool() const;
};

} // namespace fd