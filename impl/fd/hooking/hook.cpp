#include "hook.h"

#include <spdlog/spdlog.h>

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

struct _hook_enabled
{
    fd::hook* impl;
};

template <>
struct fmt::formatter<_hook_enabled> : formatter<string_view>
{
    auto format(_hook_enabled hook, format_context& ctx) const
    {
        string_view str;
        if (!hook.impl->initialized())
            str = "not created";
        else if (hook.impl->active())
            str = "already hooked";
        else
            str = "enable error";
        return formatter<string_view>::format(str, ctx);
    }
};

struct _hook_disabled
{
    fd::hook* impl;
};

template <>
struct fmt::formatter<_hook_disabled> : formatter<string_view>
{
    auto format(_hook_disabled hook, format_context& ctx) const
    {
        string_view str;
        if (!hook.impl->initialized())
            str = "not created";
        else if (!hook.impl->active())
            str = "already unhooked";
        else
            str = "disable error";
        return formatter<string_view>::format(str, ctx);
    }
};

namespace fd
{
hook::hook(std::string&& name)
    : entry_(nullptr)
    , name_(std::move(name))
{
}

hook::~hook()
{
    if (initialized())
    {
        if (active())
            disable();
        subhook_free(entry_);
        spdlog::info("{}: destroyed", name_);
    }
}

hook::hook(hook&& other) noexcept
    : entry_(std::exchange(other.entry_, nullptr))
    , name_(std::move(other.name_))
{
}

bool hook::enable()
{
    auto const ok = subhook_install(entry_) == 0;
    if (ok)
        spdlog::info("{}: hooked", name_);
    else
        spdlog::warn("{}: {}", name_, _hook_enabled(this));
    return ok;
}

bool hook::disable()
{
    auto const ok = subhook_remove(entry_) == 0;
    if (ok)
        spdlog::info("{}: unhooked", name_);
    else
        spdlog::warn("{}: {}", name_, _hook_disabled(this));
    return ok;
}

char const* hook::name() const
{
    return name_.data();
}

std::string_view hook::native_name() const
{
    return name_;
}

bool hook::initialized() const
{
    return static_cast<bool>(entry_);
}

bool hook::active() const
{
    return subhook_is_installed(entry_) != 0;
}

void* hook::get_original_method() const
{
    return subhook_get_trampoline(entry_);
}

/* void* hook::get_target_method() const
{
    return subhook_get_src(entry_);
}

void* hook::get_replace_method() const
{
    return subhook_get_dst(entry_);
} */

bool hook::init(void* target, void* replace)
{
    if (initialized())
    {
        spdlog::critical("{}: already initialized...");
        return false;
    }

    auto const entry = subhook_new(target, replace);

    if (!entry)
    {
        spdlog::error("{}: init error", name_);
        return false;
    }
    if (!subhook_get_trampoline(entry))
    {
        spdlog::error("{}: unsupported function", name_);
        subhook_free(entry);
        return false;
    }

    entry_ = entry;
    spdlog::info("{}: initialized. (target: {:p} replace: {:p})", name_, target, replace);
    return true;
}

hook::operator bool() const
{
    return initialized();
}
} // namespace fd