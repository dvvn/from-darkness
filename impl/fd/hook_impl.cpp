#include <fd/assert.h>
#include <fd/functional.h>
#include <fd/hook_impl.h>
#include <fd/log.h>

#include <subhook.h>

#define _SUBHOOK_WRAP(_FN_)                                 \
    static auto subhook_##_FN_(void* ptr)                   \
    {                                                       \
        return subhook_##_FN_(static_cast<subhook_t>(ptr)); \
    }

#undef free

// ReSharper disable All
_SUBHOOK_WRAP(free);
_SUBHOOK_WRAP(install);
_SUBHOOK_WRAP(remove);
_SUBHOOK_WRAP(is_installed);
_SUBHOOK_WRAP(get_trampoline);
_SUBHOOK_WRAP(get_src);
_SUBHOOK_WRAP(get_dst);

static void* subhook_new(void* target, void* replace)
{
    return subhook_new(target, replace, static_cast<subhook_flags_t>(subhook::HookFlagTrampoline));
}

// ReSharper restore All

#undef _SUBHOOK_WRAP

using namespace fd;

template <typename T>
concept invocable_simple = requires(T msg) { msg(); };

template <typename T>
static void _log(auto* hook, T msg)
{
    if (!log_active())
        return;

    auto hookName = hook->name();
    if (hookName.empty())
        return;

    string buff;
    if constexpr (invocable_simple<T>)
        write_string(buff, hookName, ": ", msg());
    else
        write_string(buff, hookName, ": ", msg);

    log_unsafe(buff);
}

#if 0
static class : public hook_global_callback
{
    std::mutex            mtx_;
    hook_global_callback* cb_ = nullptr;

  public:
    void store(hook_global_callback* ptr)
    {
        const std::lock_guard lock(mtx_);
        cb_ = ptr;
    }

    void construct(basic_hook* caller) override
    {
        const std::lock_guard lock(mtx_);
        if (cb_)
            cb_->construct(caller);
    }

    void destroy(const basic_hook* caller, bool unhooked) override
    {
        const std::lock_guard lock(mtx_);
        if (cb_)
            cb_->destroy(caller, unhooked);
    }

} _HookCallback;

void hook_global_callback::set(hook_global_callback* callback)
{
    _HookCallback.store(callback);
}
#endif

hook_impl::hook_impl(string_view name)
    : entry_(nullptr)
    , name_(name)
{
    //_HookCallback.construct(this);
}

hook_impl::~hook_impl()
{
    if (!hook_impl::initialized())
        return;
    if (hook_impl::active())
    {
        const auto ok = subhook_remove(entry_) == 0;
        _log(this, ok ? "unhooked" : "unhook error!");
        // _HookCallback.destroy(this, ok);
    }
    subhook_free(entry_);
}

hook_impl::hook_impl(hook_impl&& other) noexcept
    : entry_(std::exchange(other.entry_, nullptr))
    , name_(other.name_)
{
}

bool hook_impl::enable()
{
    const auto ok = subhook_install(entry_) == 0;
    _log(this, [=] {
        if (ok)
            return "hooked";
        if (!initialized())
            return "not created";
        if (active())
            return "already hooked";
        return "enable error!";
    });
    return ok;
}

bool hook_impl::disable()
{
    const auto ok = subhook_remove(entry_) == 0;
    _log(this, [=] {
        if (ok)
            return "unhooked";
        if (!initialized())
            return "not created";
        if (!active())
            return "already unhooked";
        return "unhook error!";
    });
    return ok;
}

string_view hook_impl::name() const
{
    return name_;
}

bool hook_impl::initialized() const
{
    return static_cast<bool>(entry_);
}

bool hook_impl::active() const
{
    return subhook_is_installed(entry_) != 0;
}

void* hook_impl::get_original_method() const
{
    return subhook_get_trampoline(entry_);
}

/* void* hook_impl::get_target_method() const
{
    return subhook_get_src(entry_);
}

void* hook_impl::get_replace_method() const
{
    return subhook_get_dst(entry_);
} */

void hook_impl::init(void* target, void* replace)
{
    FD_ASSERT(entry_ == nullptr);
    entry_ = subhook_new(target, replace);

    if (!entry_)
    {
        _log(this, "init error!");
    }
    else if (!subhook_get_trampoline(entry_))
    {
        _log(this, "unsupported function");
        subhook_free(entry_);
        entry_ = nullptr;
    }
}

hook_impl::operator bool() const
{
    return initialized();
}