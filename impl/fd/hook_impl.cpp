#include <fd/assert.h>
#include <fd/hook_impl.h>
#include <fd/logger.h>
#include <fd/utility.h>

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

hook_impl::~hook_impl()
{
    if (!entry_)
        return;
    if (!hook_impl::disable())
        return;
    subhook_free(entry_);
}

hook_impl::hook_impl(const string_view name)
    : entry_(nullptr)
    , name_(name)
{
}

hook_impl::hook_impl(hook_impl&& other) noexcept
    : entry_(std::exchange(other.entry_, nullptr))
    , name_(std::move(other.name_))
{
}

template <typename M>
static void _log(const hook_impl* h, const M msg)
{
    invoke(Logger, "{}: {}", bind_front(&hook_impl::name, h), msg);
}

bool hook_impl::enable()
{
    const auto ok = subhook_install(entry_) == 0;
    _log(this, [=] {
        if (ok)
            return "hooked";
        if (!entry_)
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
        if (!entry_)
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
        _log(this, "creating error!");
}

hook_impl::operator bool() const
{
    return initialized();
}
