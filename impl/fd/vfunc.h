#pragma once

#include "call_type.h"
#include "core.h"
#include "magic_cast_base.h"

#include <concepts>
#include <utility>

#undef cdecl

namespace fd
{
size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, call_type_t call);

template <typename T>
void ***get_vtable(T *instance)
{
    return static_cast<void ***>(remove_const(instance));
}

inline void ***get_vtable(void *instance)
{
    return static_cast<void ***>(instance);
}
#if 0
template <typename Fn>
size_t get_vfunc_index(void *instance, size_t vtable_offset, Fn function)
{
    /*static_assert(member_function<Fn>);*/
    return get_vfunc_index(instance, vtable_offset, get_function_pointer(function), get_call_type(function));
}
#endif

template <typename Fn>
void *get_vfunc(void *instance, size_t vtable_offset, Fn table_function)
{
    auto function_raw   = get_function_pointer(table_function);
    auto call_type      = get_call_type(table_function);
    auto function_index = get_vfunc_index(instance, vtable_offset, function_raw, call_type);

    return get_vtable(instance)[vtable_offset][function_index];
}

inline void *get_vfunc(void *instance, size_t vtable_offset, size_t function_index)
{
    return get_vtable(instance)[vtable_offset][function_index];
}

//#define GET_VFUNC_INDEX(call__, __call,call)                                                              \
//    template <typename Ret, typename T, typename... Args>                                            \
//    size_t get_vfunc_index(void *instance, size_t vtable_offset, Ret (__call T::*function)(Args...)) \
//    {                                                                                                \
//        return get_vfunc_index(instance, vtable_offset, member_function_pointer(function), call__);  \
//    }
//
// X86_CALL_MEMBER(GET_VFUNC_INDEX);
// #undef GET_VFUNC_INDEX

template <typename T>
constexpr call_type_t vtable_call = call_type_t::thiscall_;

template <typename T>
struct return_type_t
{
    using value_type = T;
};

template <typename T>
constexpr return_type_t<T> return_type;

using unknown_return_type = return_type_t<nullptr_t>;

#define VFUNC_BASE            \
    union                     \
    {                         \
        T *instance_;         \
        void *instance_void_; \
    };                        \
    void *function_;          \
                              \
  public:                     \
    void *get() const         \
    {                         \
        return function_;     \
    }

template <call_type_t Call, typename Ret, typename T, typename... Args>
class vfunc
{
    using invoker = member_func_invoker<Call, Ret, Args...>;

    VFUNC_BASE;

    template <same_call_type<Call> Fn>
    vfunc(T *instance, size_t table_offset, Fn function)
        : instance_(instance)
        , function_(get_vfunc(instance_void_, table_offset, function))
    {
        static_assert(std::invocable<Fn, T *, Args...>);
    }

    Ret operator()(Args... args) const
    {
        return invoker::call(instance_void_, function_, (args)...);
    }
};

template <typename From, typename To>
struct auto_cast_resolver;

template <typename To, call_type_t Call, typename... Args>
struct auto_cast_resolver<vfunc<Call, Args...>, To>
{
    To operator()(vfunc<Call, Args...> from) const
    {
        return magic_cast<void *, To>(from.get());
    }
};

template <call_type_t Call, typename... Args>
struct auto_cast_resolver<vfunc<Call, Args...>, void *>
{
    void *operator()(vfunc<Call, Args...> from) const
    {
        return (from.get());
    }
};

template <typename Fn>
class vfunc_wrapped_invoker
{
    Fn fn_;

  public:
    vfunc_wrapped_invoker(Fn fn)
        : fn_(std::move(fn))
    {
    }

    template <typename Ret>
    operator Ret()
    {
        static_assert(!std::is_reference_v<Ret>, "Not implemented");
        return fn_(return_type<Ret>);
    }
};

template <typename Fn>
class vfunc_wrapped_invoker<Fn &>;

template <call_type_t Call, typename T>
class vfunc<Call, unknown_return_type, T>
{
    template <typename Ret, typename... Args>
    using invoker = member_func_invoker<Call, Ret, Args...>;

    VFUNC_BASE;

    template <typename Ret, typename... Args>
    auto get() const -> build_member_func<Call, Ret, T, Args...>
    {
        return member_func_caster<Call, Ret, T, Args...>::get(function_);
    }

    template <same_call_type<Call> Fn>
    vfunc(T *instance, size_t table_offset, Fn function)
        : instance_(instance)
        , function_(get_vfunc(instance_void_, table_offset, function))
    {
    }

    vfunc(T *instance, size_t table_offset, size_t function_index)
        : instance_(instance)
        , function_(get_vfunc(instance_void_, table_offset, function_index))
    {
    }

    template <typename Ret, typename... Args>
    Ret operator()(return_type_t<Ret>, Args... args) const
    {
        return invoker<Ret, Args...>::call(instance_void_, function_, args...);
    }

    template <call_type_t Call_1>
    auto operator()(call_type_holder<Call_1>, auto...) const = delete;

    template <typename... Args>
    auto operator()(Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Ret, Args...>::call(instance_void_, function_, args...);
        });
    }
};

template <typename Ret, typename T, typename... Args>
class vfunc<call_type_t::unknown, Ret, T, Args...>
{
    using invoker = member_func_invoker<call_type_t::unknown, Ret, Args...>;

    VFUNC_BASE;

    template <typename Fn>
    vfunc(T *instance, size_t table_offset, Fn function)
        : instance_(instance)
        , function_(get_vfunc(instance_void_, table_offset, function))
    {
        static_assert(std::invocable<Fn, T *, Args...>);
    }

    template <call_type_t Call>
    Ret operator()(call_type_holder<Call> call, Args... args) const
    {
        return invoker::call(instance_void_, function_, call, args...);
    }
};

template <typename T>
class vfunc<call_type_t::unknown, unknown_return_type, T>
{
    template <typename Ret, typename... Args>
    using invoker = member_func_invoker<call_type_t::unknown, Args...>;

    VFUNC_BASE;

    template <call_type_t Call, typename Ret, typename... Args>
    auto get(call_type_holder<Call>) const -> build_member_func<Call, Ret, T, Args...>
    {
        return member_func_caster<Call, Ret, T, Args...>::get(function_);
    }

    template <typename Fn>
    vfunc(T *instance, size_t table_offset, Fn function)
        : instance_(instance)
        , function_(get_vfunc(instance_void_, table_offset, function))
    {
    }

    template <call_type_t Call, typename Ret, typename... Args>
    Ret operator()(call_type_holder<Call> call, return_type_t<Ret>, Args... args) const
    {
        return invoker<Ret, Args...>::call(instance_void_, function_, call, args...);
    }

    template <call_type_t Call, typename... Args>
    auto operator()(call_type_holder<Call> call, Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Ret, Args...>::call(instance_void_, function_, call, args...);
        });
    }
};

#undef VFUNC_BASE

template <typename T>
vfunc(T *instance, size_t table_offset, size_t func_index) -> vfunc<call_type_t::unknown, unknown_return_type, T>;

#define VFUNC_T(call__, __call, call)                     \
    template <typename Ret, typename T, typename... Args> \
    vfunc(T *instance, size_t table_offset, Ret (__call T::*func)(Args...)) -> vfunc<call__, Ret, T, Args...>;

X86_CALL_MEMBER(VFUNC_T);
#undef VFUNC_T

struct vtable_backup : boost::noncopyable
{
    using table_pointer = void **;

  private:
    table_pointer *target_;
    table_pointer backup_;

  public:
    ~vtable_backup()
    {
        if (!target_)
            return;
        *target_ = backup_;
    }

    template <typename T>
    vtable_backup(T *instance, size_t vtable_offset)
        : target_(get_vtable(instance) + vtable_offset)
        , backup_(*target_)
    {
    }

    vtable_backup(table_pointer *target)
        : target_(target)
        , backup_(*target)
    {
    }

    vtable_backup(vtable_backup &&other) noexcept
        : target_(std::exchange(other.target_, nullptr))
        , backup_(other.backup_)
    {
    }

    vtable_backup &operator=(vtable_backup &&other) noexcept
    {
        using std::swap;
        swap(target_, other.target_);
        swap(backup_, other.backup_);
        return *this;
    }

    table_pointer get() const
    {
        return backup_;
    }

    table_pointer release()
    {
        target_ = nullptr;
        return backup_;
    }
};

template <typename>
constexpr bool always_false = false;

template <typename T>
struct basic_vtable
{
    template <typename>
    friend struct vtable;

    using instance_pointer = T *;
    using table_pointer    = void **;

  private:
    union
    {
        instance_pointer instance_;
        table_pointer *vtable_;
    };

    size_t vtable_offset_;

    void const_gap()
    {
        if constexpr (always_false<T>)
            vtable_ = nullptr;
    }

  public:
    template <std::convertible_to<instance_pointer> Q = instance_pointer>
    basic_vtable(Q instance = nullptr, size_t vtable_offset = 0)
        : instance_(instance)
        , vtable_offset_(vtable_offset)
    {
    }

    template <typename From, typename To>
    basic_vtable(magic_cast<From, To> val, size_t vtable_offset = 0)
        : instance_((val))
        , vtable_offset_(vtable_offset)
    {
    }

    instance_pointer operator->() const
    {
        return instance_;
    }

    instance_pointer instance() const
    {
        return instance_;
    }

    operator instance_pointer() const
    {
        return instance_;
    }

    /*instance_pointer operator->() const
    {
        return instance_;
    }*/

    /*operator table_pointer() const
    {
        return vtable_[vtable_offset_];
    }*/

    table_pointer get() const
    {
        return vtable_[vtable_offset_];
    }

    void set(table_pointer pointer)
    {
        const_gap();
        vtable_[vtable_offset_] = pointer;
    }

    vtable_backup replace(table_pointer pointer)
    {
        const_gap();
        auto backup             = vtable_backup(vtable_ + vtable_offset_);
        vtable_[vtable_offset_] = pointer;
        return std::move(backup);
    }

    template <typename Q>
    vtable_backup replace(basic_vtable<Q> other)
    {
        return replace(other.get());
    }

    vtable_backup replace(auto) && = delete;

    vfunc<vtable_call<T>, unknown_return_type, T> operator[](size_t index) const
    {
        return {instance_, vtable_offset_, index};
    }
};

template <typename T>
struct vtable : basic_vtable<T>
{
    using basic_vtable<T>::basic_vtable;
    using basic_vtable<T>::operator[];
    /*template <typename From, typename To>
    vtable &operator=(magic_cast<From, To> val)
    {
        instance_holder<T>::operator=(static_cast<T *>(val));
        return *this;
    }*/

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename... Args>                                       \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {basic_vtable<T>::instance_, basic_vtable<T>::vtable_offset_, func}; \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);

#undef VFUNC_ACCESS
};

template <>
struct vtable<void> : basic_vtable<void>
{
    using basic_vtable::basic_vtable;
    using basic_vtable::operator[];

    /*template <member_function Fn>
    auto operator[](Fn fn) const
    {
        return vfunc(instance_, vtable_offset_, fn);
    }*/

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename T, typename... Args>                           \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {instance_, vtable_offset_, func};                                   \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);

#undef VFUNC_ACCESS

    /*template <typename Ret, typename T, typename... Args>
    vfunc<call_type_t::thiscall__, Ret, T, Args...> operator[](Ret(__thiscall *func)(void *, Args...)) const
    {
        return {instance_, vtable_offset_, func};
    }*/
};

template <typename T>
struct vtable<T *>;

template <typename T>
struct vtable<T &>;

template <typename T>
struct vtable<T const>;

template <typename T>
vtable(T *, size_t = 0) -> vtable<T>;

template <typename From, typename To>
vtable(magic_cast<From, To *>) -> vtable<To>;

template <typename From>
vtable(magic_cast<From, auto_cast_tag>) -> vtable<void>;

// template <typename To>
// vtable(magic_cast<auto_cast_tag, To>) -> vtable<std::remove_pointer_t<To>>;

// template <typename T>
// class vtable<T *> : public vtable<T>
//{
//   public:
//     using vtable<T>::vtable;
//     using vtable<T>::operator=;
// };
//
// template <typename T>
// class vtable<T **>
//{
//   public:
//     vtable(...) = delete;
// };

// template <typename T>
// vtable(T instance) -> vtable<std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, void>>;

template <typename T>
vtable(vtable<T>) -> vtable<void **>; // deleted

template <typename T>
struct cast_helper;

template <typename From, typename To>
vtable(magic_cast<From, cast_helper<To *>>) -> vtable<To>;

template <typename From, typename To>
magic_cast(vtable<From>, To) -> magic_cast<From *, To>;

// template <typename From, typename To>
// magic_cast(vtable<From *>, To) -> magic_cast<From *, To>;

template <call_type_t Call, typename Ret, typename T, typename To, typename... Args>
magic_cast(vfunc<Call, Ret, T, Args...>, To) -> magic_cast<T *, To>;

// template <typename To>
// magic_cast(vfunc_holder, To) -> magic_cast<void *, To>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd