module;

#include <fd/assert.h>
#include <fd/object.h>

#include <vector>

module fd.hooks_loader;

class hooks_loader_impl : public basic_hooks_loader
{
    std::vector<hook_base*> hooks_;

  public:
    /* ~hooks_loader_impl() override
    {
        hooks_loader_impl::disable_all();
    } */

    void disable() override
    {
        for (auto h : hooks_)
            h->disable();
    }

    bool enable(const bool stop_on_error) override
    {
        bool ok = true;

        for (auto h : hooks_)
        {
            if (h->active())
                continue;
            if (!h->initialized())
                h->init();
            if (h->enable())
                continue;
            ok = false;
            if (stop_on_error)
                break;
        }

        return ok;
    }

    void store(hook_base* const hook) override
    {
        for (const auto h : hooks_)
        {
            if (h == hook)
                return;
        }

        hooks_.push_back(hook);
    }
};

FD_OBJECT_BIND_TYPE(hooks_loader, hooks_loader_impl);
