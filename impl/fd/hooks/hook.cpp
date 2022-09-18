module;

#include <fd/assert.h>
#include <fd/utility.h>

#include <subhook.h>

module fd.hooks.impl;
import fd.logger;
import fd.functional.bind;

#define _SUBHOOK_WRAP(_FN_)                                 \
    static auto subhook_##_FN_(void* ptr)                   \
    {                                                       \
        return subhook_##_FN_(static_cast<subhook_t>(ptr)); \
    }

_SUBHOOK_WRAP(free);
_SUBHOOK_WRAP(install);
_SUBHOOK_WRAP(remove);
_SUBHOOK_WRAP(is_installed);
_SUBHOOK_WRAP(get_trampoline);
_SUBHOOK_WRAP(get_src);
_SUBHOOK_WRAP(get_dst);

#undef _SUBHOOK_WRAP

using namespace fd;
using namespace hooks;

impl::~impl()
{
    if (!entry_)
        return;
    subhook_remove(entry_); // must be done before, added for safety
    subhook_free(entry_);
}

impl::impl()
    : entry_(nullptr)
{
}

impl::impl(impl&& other)
    : entry_(std::exchange(other.entry_, nullptr))
{
}

template <typename M>
static void _Log(const impl* h, const M msg)
{
    invoke(logger, "{}: {}", bind_front(&impl::name, h), msg);
}

bool impl::enable()
{
    const auto ok = subhook_install(entry_) == 0;
    _Log(this, [=] {
        if (ok)
            return "hooked";
        if (!entry_)
            return "not created";
        if (this->active())
            return "already hooked";
        return "enable error!";
    });
    return ok;
}

bool impl::disable()
{
    const auto ok = subhook_remove(entry_) == 0;
    _Log(this, [=] {
        if (ok)
            return "unhooked";
        if (!entry_)
            return "not created";
        if (!this->active())
            return "already unhooked";
        return "unhook error!";
    });
    return ok;
}

bool impl::initialized() const
{
    return static_cast<bool>(entry_);
}

bool impl::active() const
{
    return !!subhook_is_installed(entry_);
}

void* impl::get_original_method() const
{
    return subhook_get_trampoline(entry_);
}

/* void* impl::get_target_method() const
{
    return subhook_get_src(entry_);
}

void* impl::get_replace_method() const
{
    return subhook_get_dst(entry_);
} */

void impl::init(const function_getter target, const function_getter replace)
{
    FD_ASSERT(entry_ == nullptr);
    entry_ = subhook_new(target, replace, static_cast<subhook_flags_t>(subhook::HookNoFlags));
    if (!entry_)
        _Log(this, "creating error!");
}

impl::operator bool() const
{
    return this->initialized();
}
