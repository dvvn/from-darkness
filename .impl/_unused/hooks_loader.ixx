module;

#include <typeinfo>

export module fd.hooks.loader;
export import fd.hooks.base;
export import fd.smart_ptr.shared;

using namespace fd;
using namespace hooks;

using shared_hook = shared_ptr<base>;

struct basic_loader
{
    virtual ~basic_loader() = default;

    virtual void disable() = 0;
    virtual bool enable()  = 0;

    virtual shared_hook get(const std::type_info& info) const = 0;
    virtual void store(shared_hook&& hook)                    = 0;
};

template <template <typename...> class Storage>
class loader_impl : public basic_loader
{
    using storage_type = Storage<shared_hook>;
    storage_type hooks_;

  public:
    storage_type* operator->()
    {
        return &hooks_;
    }

    const storage_type* operator->() const
    {
        return &hooks_;
    }

    void disable() override
    {
        for (auto& h : hooks_)
            h->disable();
    }

    bool enable() override
    {
        for (auto& h : hooks_)
        {
            if (h->active())
                continue;
            if (!h->initialized())
                return false;
            if (h->enable())
                continue;
            return false;
        }

        return true;
    }

    shared_hook get(const std::type_info& info) const override
    {
        for (auto& h : hooks_)
        {
            if (h.type() == info)
                return h;
        }
        return {};
    }

    void store(shared_hook&& hook) override
    {
        for (const auto& h : hooks_)
        {
            if (h.get() == hook.get())
                return;
        }

        hooks_.push_back(std::move(hook));
    }
};

export namespace fd::hooks
{
    using ::shared_hook;

    basic_loader* loader;

    using ::basic_loader;
    using ::loader_impl;

} // namespace fd::hooks
