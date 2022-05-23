module;

#include <cheat/core/object.h>

#include <future>

export module cheat.hooks.loader;
import cheat.hooks.base;

struct basic_instance_info
{
    virtual ~basic_instance_info() = default;

    virtual cheat::hooks::base* get() const noexcept = 0;
    virtual bool initialized() const noexcept = 0;

    cheat::hooks::base* operator->() const noexcept
    {
        return get();
    }
};

template <class HookInterface>
struct instance_info final : basic_instance_info
{
    cheat::hooks::base* get() const noexcept override
    {
        return &CHEAT_OBJECT_GET(HookInterface, 0);
    }

    bool initialized() const noexcept override
    {
        return CHEAT_OBJECT_GET(HookInterface, 0).initialized();
    }
};

using instance_info_ptr = std::unique_ptr<basic_instance_info>;

struct basic_hooks_loader
{
    virtual ~basic_hooks_loader() = default;

    virtual std::future<bool> start() = 0;
    virtual void stop() = 0;
    virtual void add(instance_info_ptr&& info) = 0;

    template <class HookInterface>
    void add()
    {
        add(std::make_unique<instance_info<HookInterface>>());
    }
};

constexpr size_t _Loader_idx = 0;

export namespace cheat::hooks
{
    CHEAT_OBJECT(loader, basic_hooks_loader, _Loader_idx);
}
