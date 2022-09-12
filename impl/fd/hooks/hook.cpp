module;

#include <fd/assert.h>
#include <fd/utility.h>

#include <subhook.h>

module fd.hooks.impl;
import fd.logger;
import fd.functional.bind;

using namespace fd;
using namespace hooks;

class hook_entry
{
    subhook_t hook_;

  public:
    ~hook_entry()
    {
        if (!hook_)
            return;
        subhook_remove(hook_); // must be done before, added for safety
        subhook_free(hook_);
    }

    bool create(void* target, void* replace)
    {
        FD_ASSERT(hook_ == nullptr);
        hook_ = subhook_new(target, replace, static_cast<subhook_flags_t>(subhook::HookNoFlags));
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

#if 0
impl::~impl()
{
    // purecall here
    // impl::disable( );
}
#else
impl::~impl() = default;
#endif

impl::impl()
{
    entry_ = new hook_entry();
}

impl::impl(impl&&) = default;

template <typename M>
static void _Log(const impl* h, const M msg)
{
    invoke(logger, "{}: {}", bind_front(&impl::name, h), msg);
}

bool impl::enable()
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

bool impl::disable()
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

bool impl::initialized() const
{
    return entry_->created();
}

bool impl::active() const
{
    return entry_->enabled();
}

void* impl::get_original_method() const
{
    return entry_->get_original_method();
}

void impl::init(const function_getter target, const function_getter replace)
{
    if (!entry_->create(target, replace))
    {
        _Log(this, "creating error!");
    }
}

impl::operator bool() const
{
    return static_cast<bool>(entry_);
}
