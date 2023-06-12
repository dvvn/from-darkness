#pragma once

#include "basic_hook.h"
#include "call_type.h"
#include "core.h"
#include "tool/vector.h"
#ifdef _DEBUG
#include "tool/string.h"
#else
#include "tool/string_view.h"
#endif

namespace fd
{
template <typename Callback>
Callback *unique_hook_callback;

template <typename Callback>
void *unique_hook_trampoline;

template <class Callback, call_type_t Call, typename Ret, class C, typename... Args>
struct hook_proxy_member;

// template <class Callback, call_type_t Call, typename Ret, class C, typename... Args>
// struct hook_proxy_member<Callback &, Call, Ret, C, Args...>
//     : hook_proxy_member<Callback, Call, Ret, C, Args...>
//{
// };

template <call_type_t Call, typename Ret, class C, typename... Args>
class hook_proxy_member_holder
{
    using builder = member_func_builder<Call, Ret, void, Args...>;

    void *original_;

    union
    {
        void *proxy_;
        C *this_;
    };

  public:
    hook_proxy_member_holder(void *original, void *proxy)
        : original_(original)
        , proxy_(proxy)
    {
    }

    Ret operator()(Args... args)
    {
        return builder::invoke(original_, this_, args...);
    }

    operator C *()
    {
        return this_;
    }

    C *operator->()
    {
        return this_;
    }
};

template <class Callback, call_type_t Call, typename Ret, class Hooked, typename... Args>
Ret invoke_hook_proxy(hook_proxy_member<Callback, Call, Ret, Hooked, Args...> *proxy, Args... args)
{
    using proxy_holder = hook_proxy_member_holder<Call, Ret, Hooked, Args...>;
    auto &callback     = *unique_hook_callback<Callback>;
    if constexpr (std::invocable<Callback, hook_proxy_member<Callback, Call, Ret, Hooked, Args...> &, Args...>)
        return callback(*proxy, args...);
    else
        return callback(proxy_holder(unique_hook_trampoline<Callback>, proxy), args...);
}

template <class Callback, call_type_t Call, typename Ret, class Hooked, typename... Args>
Ret invoke_hook_original(hook_proxy_member<Callback, Call, Ret, Hooked, Args...> *proxy, Args... args)
{
    return member_func_builder<Call, Ret, void, Args...>::invoke(unique_hook_trampoline<Callback>, proxy, args...);
}

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                             \
    template <class Callback, typename Ret, class C, typename... Args>        \
    struct hook_proxy_member<Callback, call__, Ret, C, Args...> : noncopyable \
    {                                                                         \
        Ret operator()(Args... args)                                          \
        {                                                                     \
            return invoke_hook_original(this, args...);                       \
        }                                                                     \
        operator C *()                                                        \
        {                                                                     \
            return reinterpret_cast<C *>(this);                               \
        }                                                                     \
        C *operator->()                                                       \
        {                                                                     \
            return reinterpret_cast<C *>(this);                               \
        }                                                                     \
        Ret __call proxy(Args... args) noexcept                               \
        {                                                                     \
            return invoke_hook_proxy(this, args...);                          \
        }                                                                     \
    };

X86_CALL_MEMBER(HOOK_PROXY_MEMBER);
#undef HOOK_PROXY_MEMBER

// template <class Callback, call_type_t Call, typename Ret, class C, typename... Args>
// auto hook_proxy_member<Callback, Call, Ret, C, Args...>::operator()(Args... args) -> Ret
//{
//     return member_func_builder<Call, Ret, C, Args...>::invoke(this, unique_hook_trampoline<Callback>, args...);
// }

template <class Callback, call_type_t Call, typename Ret, typename... Args>
struct hook_proxy;

/*template <class Callback, typename Ret, typename C, typename... Args>
struct hook_proxy<Callback, call_type_t::thiscall__, Ret, C, Args...>
    : hook_proxy_member<Callback, call_type_t::thiscall__, Ret, C, Args...>
{
};*/

template <class Callback, call_type_t Call, typename Ret, typename... Args>
Ret invoke_hook_proxy(Args... args) noexcept
{
    // WIP
}

#define HOOK_PROXY_STATIC(call__, __call, call)                                                  \
    template <class Callback, typename Ret, typename... Args>                                    \
    struct hook_proxy<Callback, call__, Ret, Args...>                                            \
    {                                                                                            \
        static Ret __call proxy(Args... args) noexcept                                           \
        {                                                                                        \
            using function_type = Ret(__call *)(Args...);                                        \
            auto &callback      = *unique_hook_callback<Callback>;                               \
            auto original       = void_to_func<function_type>(unique_hook_trampoline<Callback>); \
            return callback(original, std::forward<Args>(args)...);                              \
        }                                                                                        \
    };

X86_CALL(HOOK_PROXY_STATIC);
#undef HOOK_PROXY_STATIC

//----------------

struct basic_hook_proxy : basic_hook
{
    bool enable() final;
    bool disable() final;
};

struct basic_hook_lazy_proxy : basic_hook_lazy
{
    bool enable() final;
    bool disable() final;
};

using hook_name = char const *;

class basic_hook_data : public basic_hook_proxy, public basic_hook_lazy_proxy
{
#ifdef _DEBUG
    string name_;
#else
    static constexpr string_view name_ = "Unknown";
#endif
#if __has_include(<minhook.h>)
    void *target_;
#elif __has_include(<subhook.h>)

#endif

  public:
    struct hook_info info() const;

    basic_hook_data(hook_name name, void *target);
    basic_hook_lazy *lazy() final;

    char const *name() const final;
};

template <typename Callback>
class hook_data final : public basic_hook_data
{
    Callback callback_;

  public:
    hook_data(hook_name name, void *target, Callback &callback)
        : basic_hook_data(name, target)
        , callback_(std::move(callback))
    {
    }

    Callback *callback()
    {
        return &callback_;
    }
};

template <call_type_t Call, typename Ret, typename T, typename... Args>
class vfunc;

class hook_context : public noncopyable
{
    using storage_type = vector<basic_hook *>;
    // using error_handler = std::function<void(void *)>;

    storage_type storage_;

    void *create_trampoline(hook_name name, void *target, void *replace);

    template <typename Callback>
    basic_hook *store(hook_name name, void *target, void *trampoline, Callback &callback)
    {
        auto hook = new hook_data<Callback>(name, target, callback);
        storage_.emplace_back(hook);
        unique_hook_trampoline<Callback> = trampoline;
        unique_hook_callback<Callback>   = hook->callback();
        return hook;
    }

    template <class Proxy, typename Callback>
    basic_hook *do_create(hook_name name, void *target, Callback &callback) noexcept
    {
        auto trampoline = create_trampoline(name, target, get_function_pointer(&Proxy::proxy));
        return trampoline ? store(name, target, trampoline, callback) : nullptr;
    }

  public:
    ~hook_context();
    hook_context();

    bool enable();
    bool disable();

    bool enable_lazy();
    bool disable_lazy();

    size_t size() const;

#define MAKE_HOOK_CALLBACK_MEMBER(call__, __call, call)                                     \
    template <typename Callback, typename Ret, class C, typename... Args>                   \
    basic_hook *create(hook_name name, Ret (__call C::*target)(Args...), Callback callback) \
    {                                                                                       \
        using proxy_type = hook_proxy_member<Callback, call__, Ret, C, Args...>;            \
        return do_create<proxy_type>(name, get_function_pointer(target), callback);         \
    }

    X86_CALL_MEMBER(MAKE_HOOK_CALLBACK_MEMBER);
#undef MAKE_HOOK_CALLBACK

#define MAKE_HOOK_CALLBACK_STAITC(call__, __call, call)                                 \
    template <typename Callback, typename Ret, typename... Args>                        \
    basic_hook *create(hook_name name, Ret(__call *target)(Args...), Callback callback) \
    {                                                                                   \
        using proxy_type = hook_proxy<Callback, call__, Ret, Args...>;                  \
        return do_create<proxy_type>(name, get_function_pointer(target), callback);     \
    }

    X86_CALL(MAKE_HOOK_CALLBACK_STAITC);
#undef MAKE_HOOK_CALLBACK

    template <typename Callback, typename Ret, typename C, typename... Args>
    basic_hook *create(hook_name name, Ret(__thiscall *target)(C *, Args...), Callback callback)
    {
        using proxy_type = hook_proxy_member<Callback, call_type_t::thiscall_, Ret, C, Args...>;
        return do_create<proxy_type>(name, get_function_pointer(target), callback);
    }

    template <typename Callback, call_type_t Call, typename Ret, typename T, typename... Args>
    basic_hook *create(hook_name name, vfunc<Call, Ret, T, Args...> target, Callback callback)
    {
        using proxy_type = hook_proxy_member<Callback, Call, Ret, T, Args...>;
        return do_create<proxy_type>(name, target.get(), callback);
    }
};
} // namespace fd