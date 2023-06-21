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

template <class Callback, call_type_t Call_T, typename Ret, class C, typename... Args>
struct hook_proxy_member;

template <call_type_t Call_T, typename Ret, class Object, typename... Args>
class hook_proxy_member_holder
{
    void *original_;

    union
    {
        void *proxy_;
        Object *this_;
    };

  public:
    hook_proxy_member_holder(void *original, void *proxy)
        : original_(original)
        , proxy_(proxy)
    {
    }

    Ret operator()(Args... args)
    {
        member_func_invoker<Call_T, Ret, Object, Args...> invoker;
        return invoker(original_, this_, args...);
    }

    operator Object *()
    {
        return this_;
    }

    Object *operator->()
    {
        return this_;
    }
};

template <class Callback, call_type_t Call_T, typename Ret, class Object, typename... Args>
struct basic_hook_proxy_member : noncopyable
{
    using proxy_type = hook_proxy_member<Callback, Call_T, Ret, Object, Args...>;

    proxy_type *self()
    {
        return static_cast<proxy_type *>(this);
    }

    Object *get()
    {
        return reinterpret_cast<Object *>(self());
    }

    Ret operator()(Args... args)
    {
        member_func_invoker<Call_T, Ret, Object, Args...> invoker;
        return invoker(unique_hook_trampoline<Callback>, get(), args...);
    }

    operator Object *()
    {
        return get();
    }

    Object *operator->()
    {
        return get();
    }
};

template <class Callback, call_type_t Call_T, typename Ret, class Object, typename... Args>
Ret invoke_hook_proxy(hook_proxy_member<Callback, Call_T, Ret, Object, Args...> *proxy, Args... args)
{
    using proxy_holder = hook_proxy_member_holder<Call_T, Ret, Object, Args...>;
    auto &callback     = *unique_hook_callback<Callback>;
    if constexpr (std::invocable<Callback, decltype(*proxy), Args...>)
        return callback(*proxy, args...);
    else
        return callback(proxy_holder(unique_hook_trampoline<Callback>, proxy), args...);
}

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                      \
    template <class Callback, typename Ret, class C, typename... Args> \
    struct hook_proxy_member<Callback, call__, Ret, C, Args...> final  \
        : basic_hook_proxy_member<Callback, call__, Ret, C, Args...>   \
    {                                                                  \
        Ret __call proxy(Args... args) noexcept                        \
        {                                                              \
            return invoke_hook_proxy(this, args...);                   \
        }                                                              \
    };

X86_CALL_MEMBER(HOOK_PROXY_MEMBER);
#undef HOOK_PROXY_MEMBER

template <class Callback, call_type_t Call_T, typename Ret, typename... Args>
struct hook_proxy;

template <call_type_t Call_T, typename Ret, typename... Args>
class hook_proxy_holder
{
    void *original_;

  public:
    hook_proxy_holder(void *original)
        : original_(original)
    {
    }

    Ret operator()(Args... args)
    {
        non_member_func_invoker<Call_T, Ret, Args...> invoker;
        return invoker(original_, args...);
    }
};

template <class Callback, call_type_t Call_T, typename Ret, typename... Args>
Ret invoke_hook_proxy(Args... args)
{
    auto &callback = *unique_hook_callback<Callback>;
#if 1
    using raw_func = non_member_func_type<Call_T, Ret, Args...>;
    auto original  = void_to_func<raw_func>(unique_hook_trampoline<Callback>);
    return callback(original, args...);
#else
    using holder  = hook_proxy_holder<Call_T, Ret, Args...>;
    auto original = unique_hook_trampoline<Callback>;
    return callback(holder(original), args...);
#endif
}

#define HOOK_PROXY_STATIC(call__, __call, call)                       \
    template <class Callback, typename Ret, typename... Args>         \
    struct hook_proxy<Callback, call__, Ret, Args...> final           \
    {                                                                 \
        static Ret __call proxy(Args... args) noexcept                \
        {                                                             \
            return invoke_hook_proxy<Callback, call__, Ret>(args...); \
        }                                                             \
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

template <class P>
struct hook_proxy_getter
{
    static void *get()
    {
        return get_function_pointer(&P::proxy);
    }
};

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
class vfunc;
struct abstract_function_tag;

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
        auto trampoline = create_trampoline(name, target, hook_proxy_getter<Proxy>::get());
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

#define HOOK_PROXY_SAMPLE template <typename, call_type_t, typename...>

#define MAKE_HOOK_CALLBACK_MEMBER(call__, __call, call)                                     \
    template <                                                                              \
        HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member,                                  \
        typename Callback,                                                                  \
        typename Ret,                                                                       \
        class C,                                                                            \
        typename... Args>                                                                   \
    basic_hook *create(hook_name name, Ret (__call C::*target)(Args...), Callback callback) \
    {                                                                                       \
        using proxy_type = Proxy<Callback, call__, Ret, C, Args...>;                        \
        return do_create<proxy_type>(name, get_function_pointer(target), callback);         \
    }

    _X86_CALL_PROXY(MAKE_HOOK_CALLBACK_MEMBER, thiscall) X86_CALL(MAKE_HOOK_CALLBACK_MEMBER);
#undef MAKE_HOOK_CALLBACK_MEMBER

#define MAKE_HOOK_CALLBACK_STAITC(call__, __call, call)                                                      \
    template <HOOK_PROXY_SAMPLE class Proxy = hook_proxy, typename Callback, typename Ret, typename... Args> \
    basic_hook *create(hook_name name, Ret(__call *target)(Args...), Callback callback)                      \
    {                                                                                                        \
        using proxy_type = Proxy<Callback, call__, Ret, Args...>;                                            \
        return do_create<proxy_type>(name, get_function_pointer(target), callback);                          \
    }

    X86_CALL(MAKE_HOOK_CALLBACK_STAITC);
#undef MAKE_HOOK_CALLBACK_STAITC

    template <
        HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member,
        typename Callback,
        typename Ret,
        typename C,
        typename... Args>
    basic_hook *create(hook_name name, Ret(__thiscall *target)(C *, Args...), Callback callback)
    {
        using proxy_type = Proxy<Callback, call_type_t::thiscall_, Ret, C, Args...>;
        return do_create<proxy_type>(name, get_function_pointer(target), callback);
    }

    template <
        HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member,
        typename Callback,
        call_type_t Call_T,
        typename Ret,
        typename T,
        typename... Args>
    basic_hook *create(hook_name name, vfunc<Call_T, Ret, T, Args...> target, Callback &&callback)
    {
        using proxy_type = Proxy<std::remove_reference_t<Callback>, Call_T, Ret, T, Args...>;
        return do_create<proxy_type>(name, target.get(), callback);
    }

    template <
        HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member,
        typename Callback,
        std::derived_from<abstract_function_tag> Fn>
    basic_hook *create(hook_name name, Fn abstract_fn, Callback callback)
    {
        return create<Proxy>(name, abstract_fn.get(), (callback));
    }

#undef HOOK_PROXY_SAMPLE
};
} // namespace fd