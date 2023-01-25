#pragma once

#include <fd/call_cvs.h>
#include <fd/hook_impl.h>

namespace fd
{
template <typename To, typename From = void*>
static To _force_cast(From from, [[maybe_unused]] To toHint = {})
{
    static_assert(sizeof(From) == sizeof(To));

    union
    {
        From from;
        To   to;
    } helper;

    helper.from = from;
    return helper.to;
}

template <typename T>
void* decay_fn(T fn)
{
    return _force_cast<void*>(fn);
}

template <class T>
void* decay_fn(T* instance, size_t vfunc)
{
    return (*_force_cast<void***>(instance))[vfunc];
}

template <typename Fn = void*, class T>
Fn decay_fn(T& instanceHolder, size_t vfunc) requires(std::is_class_v<T>)
{
    auto fn = decay_fn(instanceHolder.get(), vfunc);
    if constexpr (!std::convertible_to<void*, Fn>)
        return _force_cast<Fn>(fn);
    else
        return fn;
}

template <typename Callback, typename Ret, call_cvs Cvs, class Class, typename... Args>
class hook_callback;

template <class Self, typename Fn>
struct hook_original_proxy
{
    Self* thisPtr;
    Fn    original;

    static_assert(sizeof(Fn) == sizeof(void*));

    template <typename... Args>
    auto operator()(Args... args)
    {
        return (*thisPtr.*original)(static_cast<Args>(args)...);
    }
};

template <typename Fn>
struct hook_original_proxy<void, Fn>
{
    Fn original;

    template <typename... Args>
    auto operator()(Args... args) const
    {
        return original(static_cast<Args>(args)...);
    }
};

template <class C, typename Fn>
hook_original_proxy(C*, Fn) -> hook_original_proxy<C, std::decay_t<Fn>>;

template <typename Fn>
hook_original_proxy(Fn) -> hook_original_proxy<void, std::decay_t<Fn>>;

#define FD_HOOK_CALLBACK_MEMBER(_CCVS_)                                                                                                                                  \
    template <typename Callback, typename Ret, class Class, typename... Args>                                                                                            \
    class hook_callback<Callback, Ret, call_cvs::_CCVS_##_, Class, Args...> : public hook_impl                                                                           \
    {                                                                                                                                                                    \
        using function_type = Ret (__##_CCVS_ Class::*)(Args...);                                                                                                        \
        friend struct hook_original_proxy<Class, function_type>;                                                                                                         \
        inline static hook_callback* self_;                                                                                                                              \
        Callback                     callback_;                                                                                                                          \
        Ret __##_CCVS_ proxy(Args... args)                                                                                                                               \
        {                                                                                                                                                                \
            return self_->callback_(                                                                                                                                     \
                hook_original_proxy(this, _force_cast(self_->get_original_method(), &hook_callback::proxy)), reinterpret_cast<Class*>(this), std::forward<Args>(args)... \
            );                                                                                                                                                           \
        }                                                                                                                                                                \
                                                                                                                                                                         \
      public:                                                                                                                                                            \
        hook_callback(string_view name, function_type target, Callback callback)                                                                                         \
            : hook_impl(name)                                                                                                                                            \
            , callback_(std::move(callback))                                                                                                                             \
        {                                                                                                                                                                \
            self_ = this;                                                                                                                                                \
            hook_impl::init(decay_fn(target), decay_fn(&hook_callback::proxy));                                                                                          \
        }                                                                                                                                                                \
        hook_callback(string_view name, function_type, void* target, Callback callback)                                                                                  \
            : hook_impl(name)                                                                                                                                            \
            , callback_(std::move(callback))                                                                                                                             \
        {                                                                                                                                                                \
            self_ = this;                                                                                                                                                \
            hook_impl::init(target, decay_fn(&hook_callback::proxy));                                                                                                    \
        }                                                                                                                                                                \
        hook_callback(hook_callback&& other)                                                                                                                             \
            : hook_impl(std::move(other))                                                                                                                                \
            , callback_(std::move(other.callback_))                                                                                                                      \
        {                                                                                                                                                                \
            self_ = this;                                                                                                                                                \
        }                                                                                                                                                                \
    };                                                                                                                                                                   \
    template <typename Callback, typename Ret, class Class, typename... Args>                                                                                            \
    hook_callback(string_view, Ret (__##_CCVS_ Class::*)(Args...), Callback) -> hook_callback<std::decay_t<Callback>, Ret, call_cvs::_CCVS_##_, Class, Args...>;         \
    template <typename Callback, typename Ret, class Class, typename... Args>                                                                                            \
    hook_callback(string_view, Ret (__##_CCVS_ Class::*)(Args...), void*, Callback) -> hook_callback<std::decay_t<Callback>, Ret, call_cvs::_CCVS_##_, Class, Args...>;

#define FD_HOOK_CALLBACK(_CCVS_)                                                                                                                         \
    template <typename Callback, typename Ret, typename... Args>                                                                                         \
    class hook_callback<Callback, Ret, call_cvs::_CCVS_##_, void, Args...> : public hook_impl                                                            \
    {                                                                                                                                                    \
        using function_type = Ret(__##_CCVS_*)(Args...);                                                                                                 \
        friend struct hook_original_proxy<void, function_type>;                                                                                          \
        inline static hook_callback* self_;                                                                                                              \
        Callback                     callback_;                                                                                                          \
        static Ret __##_CCVS_ proxy(Args... args)                                                                                                        \
        {                                                                                                                                                \
            return self_->callback_(hook_original_proxy(_force_cast(self_->get_original_method(), &hook_callback::proxy)), std::forward<Args>(args)...); \
        }                                                                                                                                                \
                                                                                                                                                         \
      public:                                                                                                                                            \
        hook_callback(string_view name, function_type target, Callback callback)                                                                         \
            : hook_impl(name)                                                                                                                            \
            , callback_(std::move(callback))                                                                                                             \
        {                                                                                                                                                \
            self_ = this;                                                                                                                                \
            this->init(decay_fn(target), decay_fn(&hook_callback::proxy));                                                                               \
        }                                                                                                                                                \
        hook_callback(string_view name, function_type, void* target, Callback callback)                                                                  \
            : hook_impl(name)                                                                                                                            \
            , callback_(std::move(callback))                                                                                                             \
        {                                                                                                                                                \
            self_ = this;                                                                                                                                \
            this->init(target, decay_fn(&hook_callback::proxy));                                                                                         \
        }                                                                                                                                                \
        hook_callback(hook_callback&& other)                                                                                                             \
            : hook_impl(std::move(other))                                                                                                                \
            , callback_(std::move(other.callback_))                                                                                                      \
        {                                                                                                                                                \
            self_ = this;                                                                                                                                \
        }                                                                                                                                                \
    };                                                                                                                                                   \
    template <typename Callback, typename Ret, typename... Args>                                                                                         \
    hook_callback(string_view, Ret(__##_CCVS_*)(Args...), Callback) -> hook_callback<std::decay_t<Callback>, Ret, call_cvs::_CCVS_##_, void, Args...>;   \
    template <typename Callback, typename Ret, typename... Args>                                                                                         \
    hook_callback(string_view, Ret(__##_CCVS_*)(Args...), void*, Callback) -> hook_callback<std::decay_t<Callback>, Ret, call_cvs::_CCVS_##_, void, Args...>;

#define FD_HOOK_CALLBACK_ANY(_CCVS_) \
    FD_HOOK_CALLBACK_MEMBER(_CCVS_); \
    FD_HOOK_CALLBACK(_CCVS_);

#undef cdecl

FD_HOOK_CALLBACK_ANY(cdecl);
FD_HOOK_CALLBACK_ANY(fastcall);
FD_HOOK_CALLBACK_ANY(stdcall);
FD_HOOK_CALLBACK_ANY(vectorcall);
FD_HOOK_CALLBACK_MEMBER(thiscall);

#undef FD_HOOK_CALLBACK_MEMBER
#undef FD_HOOK_CALLBACK
#undef FD_HOOK_CALLBACK_ANY
}