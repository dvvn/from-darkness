module;

#include <fd/assert.h>
#include <fd/utility.h>

#include <subhook.h>

module fd.hook;
import fd.logger;
import fd.functional.bind;

using namespace fd;

class hook_entry
{
    subhook_t hook_;

  public:
    ~hook_entry()
    {
        if (hook_)
            subhook_free(hook_);
    }

    bool create(void* target, void* replace)
    {
        FD_ASSERT(hook_ == nullptr);
        hook_ = subhook_new(target, replace, (subhook_flags_t)subhook::HookNoFlags);
        return hook_ != nullptr;
    }

    bool created() const
    {
        return hook_ != nullptr;
    }

    bool enabled() const
    {
        return !!subhook_is_installed(hook_);
    }

    bool enable()
    {
        return subhook_install(hook_) == 0;
    }

    bool disable()
    {
        return subhook_remove(hook_) == 0;
    }

    void* get_original_method() const
    {
        return subhook_get_trampoline(hook_);
    }

    void* get_target_method() const
    {
        return subhook_get_src(hook_);
    }

    void* get_replace_method() const
    {
        return subhook_get_dst(hook_);
    }
};

hook_impl::~hook_impl()
{
    // purecall here
    // hook_impl::disable( );
}

hook_impl::hook_impl()
{
    entry_ = new hook_entry();
}

template <typename M>
static void _Log(const hook_impl* h, const M msg)
{
    invoke(logger, "{}: {}", bind_front(&hook_impl::name, h), msg);
}

bool hook_impl::enable()
{
    if (!entry_->created())
    {
        _Log(this, "not created!");
        return false;
    }
    if (!entry_->enable())
    {
        _Log(this, [this] {
            return entry_->enabled() ? "already hooked" : "enable error!";
        });
        return false;
    }
    _Log(this, "hooked");
    return true;
}

bool hook_impl::disable()
{
    const auto ok = entry_->disable();
    _Log(this, [ok, this] {
        if (ok)
            return "unhooked";
        if (!entry_->enabled())
            return "already unhooked";
        if (entry_->created())
            return "unhook error!";
        return "not created!";
    });
    return ok;
}

bool hook_impl::initialized() const
{
    return entry_->created();
}

bool hook_impl::active() const
{
    return entry_->enabled();
}

void* hook_impl::get_original_method() const
{
    return entry_->get_original_method();
}

void hook_impl::init(const function_getter target, const function_getter replace)
{
    if (!entry_->create(target, replace))
    {
        _Log(this, "creating error!");
    }
}
