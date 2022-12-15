#include <fd/assert.h>
#include <fd/hook_impl.h>
#include <fd/logger.h>

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
// ReSharper restore All

#undef _SUBHOOK_WRAP

using namespace fd;

template <typename T>
static void _log(auto* hook, T msg)
{
    if (!Logger)
        return;
    auto hookName = hook->name();
    if (hookName.empty())
        return;
    if constexpr (invocable<T>)
        invoke(*Logger, make_string(hookName, ": ", invoke(msg)));
    else
        invoke(*Logger, make_string(hookName, ": ", msg));
}

hook_impl::hook_impl()
    : inUse_(false)
    , entry_(nullptr)
{
    if (HookGlobalCallback)
        HookGlobalCallback->construct(this);
}

hook_impl::~hook_impl()
{
    if (!inUse_)
    {
        _log(this, "never used!");
        return;
    }

    if (!hook_impl::initialized())
        return;
    if (hook_impl::active())
    {
        const auto ok = subhook_remove(entry_) == 0;
        _log(this, ok ? "unhooked" : "unhook error!");
        if (HookGlobalCallback)
            HookGlobalCallback->destroy(this, ok);
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
    inUse_ = true;
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

void hook_impl::set_name(const string_view name)
{
    name_ = name;
}

bool hook_impl::initialized() const
{
    return static_cast<bool>(entry_);
}

bool hook_impl::active() const
{
    return !!subhook_is_installed(entry_);
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

void hook_impl::init(const function_getter target, const function_getter replace)
{
    FD_ASSERT(entry_ == nullptr);
    entry_ = subhook_new(target, replace, static_cast<subhook_flags_t>(subhook::HookNoFlags));
    if (!entry_)
        _log(this, "init error!");
}

hook_impl::operator bool() const
{
    return initialized();
}

//----

hook_callback_ret_wrapper<void>::operator bool() const
{
    return value_;
}

void hook_callback_ret_wrapper<void>::emplace()
{
    value_ = true;
}

namespace fd
{
    hook_global_callback* HookGlobalCallback;
}