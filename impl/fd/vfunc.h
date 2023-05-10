#pragma once

#include "x86_call.h"

#include <concepts>
#include <utility>

namespace fd
{
template <typename From, typename To>
class magic_cast;

#if 0
class vfunc_holder
{
    void *func_;

  public:
    vfunc_holder(void *vfunc)
        : func_(vfunc)
    {
    }

    vfunc_holder(void *instance, size_t table_offset, size_t func_index)
        : func_(static_cast<void ***>(instance_internal_)[table_offset][func_index])
    {
    }

    operator void *() const
    {
        return func_;
    }

    void *get() const
    {
        return func_;
    }
};

template <typename T>
class vfunc_instance_holder
{
    T *instance_;

  public:
    vfunc_instance_holder(T *instance)
        : instance_((instance))
    {
    }

    T *instance() const
    {
        return instance_;
    }
};
#endif
// inline size_t get_vfunc_index(void *instance, size_t vtable_offset, size_t index, _x86_call call)
//{
//     return index;
// }

size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, _x86_call call);

template <typename Fn>
size_t get_vfunc_index(void *instance, size_t vtable_offset, Fn function)
{
    /*static_assert(member_function<Fn>);*/
    static_assert(sizeof(Fn) == sizeof(void *));

    union
    {
        Fn fn;
        void *fn_ptr;
    };

    fn = function;

    return get_vfunc_index(instance, vtable_offset, fn_ptr, get_call_type(function));
}

//#define GET_VFUNC_INDEX(call__, __call)                                                              \
//    template <typename Ret, typename T, typename... Args>                                            \
//    size_t get_vfunc_index(void *instance, size_t vtable_offset, Ret (__call T::*function)(Args...)) \
//    {                                                                                                \
//        return get_vfunc_index(instance, vtable_offset, member_function_pointer(function), call__);  \
//    }
//
// X86_CALL_MEMBER(GET_VFUNC_INDEX);
// #undef GET_VFUNC_INDEX

template <_x86_call Call, typename... Args>
struct vfunc_invoker;

struct vfunc_invoker_gap
{
};

#define VFUNC_INVOKER(call__, __call)                                                              \
    template <typename... Args>                                                                    \
    struct vfunc_invoker<call__, Args...>                                                          \
    {                                                                                              \
        template <typename Ret, typename... Args2>                                                 \
        static Ret call(void *instance, void *fn, Args2 &&...args) noexcept                        \
        {                                                                                          \
            union                                                                                  \
            {                                                                                      \
                void *fn1;                                                                         \
                Ret (__call vfunc_invoker_gap::*fn2)(Args...);                                     \
            };                                                                                     \
            fn1 = fn;                                                                              \
            return (*static_cast<vfunc_invoker_gap *>(instance).*fn2)(static_cast<Args>(args)...); \
        }                                                                                          \
    };

X86_CALL_MEMBER(VFUNC_INVOKER);
#undef VFUNC_INVOKER

template <typename... Args>
struct vfunc_invoker<_x86_call::unknown, Args...>
{
    template <typename Ret, typename... Args2>
    static Ret call(void *instance, void *fn, _x86_call info, Args2 &&...args) noexcept
    {
#define SELECT_INVOKER(call__, __call) \
    case call__:                       \
        return vfunc_invoker<call__, Args...>::template call<Ret>(instance, fn, std::forward<Args2>(args)...);

        switch (info)
        {
            X86_CALL_MEMBER(SELECT_INVOKER);
        default:
            std::unreachable();
        }

#undef SELECT_INVOKER
    }

    template <typename Ret, member_function Fn, typename... Args2>
    static Ret call(void *instance, Fn fn, Args2 &&...args) noexcept
    {
        return call<Ret>(instance, fn, get_call_type(fn), std::forward<Args2>(args)...);
    }
};

template <typename... Args>
using unknown_vfunc_invoker = vfunc_invoker<_x86_call::unknown, Args...>;

template <typename T>
constexpr _x86_call vtable_call = _x86_call::thiscall__;

template <typename T>
class instance_holder
{
    template <_x86_call Call, typename Ret, typename T1, typename... Args>
    friend class vfunc;

    template <typename T1>
    friend class vtable;

    union
    {
        T *instance_;
        void *instance_internal_;
    };

  public:
    instance_holder(T *instance)
        : instance_(instance)
    {
    }

    T *instance() const
    {
        return instance_;
    }

    T *operator->() const
    {
        return instance_;
    }
};

class vfunc_holder
{
    template <_x86_call Call, typename Ret, typename T, typename... Args>
    friend class vfunc;

    void *func_;

  public:
    vfunc_holder(void *func)
        : func_(func)
    {
    }

    void *get() const
    {
        return func_;
    }
};

template <_x86_call Call, typename Ret, typename T, typename... Args>
class vfunc : public instance_holder<T>, public vfunc_holder
{
    using invoker = vfunc_invoker<Call, Args...>;
    using instance_holder<T>::instance_internal_;

  public:
    vfunc(T *instance, size_t table_offset, size_t func_index)
        : instance_holder<T>(instance)
        , vfunc_holder(static_cast<void ***>(instance_internal_)[table_offset][func_index])
    {
    }

    vfunc(T *instance, size_t table_offset, auto func)
        : instance_holder<T>(instance)
        , vfunc_holder(static_cast<void ***>(
              instance_internal_)[table_offset][get_vfunc_index(instance_internal_, table_offset, func)])
    {
    }

    template <typename... Args2>
    Ret operator()(Args2 &&...args) const
    {
        return invoker::template call<Ret>(instance_internal_, func_, std::forward<Args2>(args)...);
    }
};

template <_x86_call Call, typename T>
class vfunc<Call, nullptr_t, T> : public instance_holder<T>, public vfunc_holder
{
    template <typename Ret, typename... Args>
    using invoker = vfunc_invoker<Call, Ret, T, Args...>;
    using instance_holder<T>::instance_internal_;

  public:
    vfunc(T *instance, size_t table_offset, size_t func_index)
        : instance_holder<T>(instance)
        , vfunc_holder(static_cast<void ***>(instance_internal_)[table_offset][func_index])
    {
    }

    vfunc(T *instance, size_t table_offset, auto func)
        : vfunc(instance, table_offset, get_vfunc_index(instance, table_offset, func))
    {
    }

    template <typename Ret, typename... Args>
    Ret operator()(std::in_place_type_t<Ret>, Args... args) const
    {
        return invoker<Ret, Args...>::template call<Ret>(instance_internal_, func_, static_cast<Args>(args)...);
    }
};

template <_x86_call Call, typename T>
using custom_vfunc = vfunc<Call, nullptr_t, T>;

template <typename T>
class vfunc<_x86_call::unknown, nullptr_t, T> : public instance_holder<T>, public vfunc_holder
{
    template <typename... Args>
    using invoker = unknown_vfunc_invoker<Args...>;
    using instance_holder<T>::instance_internal_;

    _x86_call call_;

  public:
    vfunc(T *instance, size_t table_offset, size_t func_index, _x86_call call = vtable_call<T>)
        : instance_holder<T>(instance)
        , vfunc_holder(static_cast<void ***>(instance_internal_)[table_offset][func_index])
        , call_(call)
    {
    }

    vfunc(T *instance, size_t table_offset, auto func)
        : vfunc(instance, table_offset, get_call_type(func), get_vfunc_index(instance, table_offset, func))
    {
    }

    template <typename Ret, typename... Args>
    Ret operator()(std::in_place_type_t<Ret>, Args... args) const
    {
        return invoker<Args...>::template call<Ret>(instance_internal_, func_, call_, static_cast<Args>(args)...);
    }
};

template <typename T>
using unknown_vfunc = vfunc<_x86_call::unknown, nullptr_t, T>;

template <typename T>
vfunc(T *instance, size_t table_offset, size_t func_index, _x86_call call)->unknown_vfunc<T>;

template <typename T>
vfunc(T *instance, size_t table_offset, size_t func_index)->unknown_vfunc<T>;

template <typename Ret, typename T, typename... Args>
class vfunc<_x86_call::unknown, Ret, T, Args...> : public instance_holder<T>, public vfunc_holder
{
    using invoker = vfunc_invoker<_x86_call::unknown, Args...>;

    _x86_call call_;
    using instance_holder<T>::instance_internal_;

  public:
    vfunc(T *instance, size_t table_offset, _x86_call call, size_t func_index)
        : instance_holder<T>(instance)
        , vfunc_holder(static_cast<void ***>(instance_internal_)[table_offset][func_index])
        , call_(call)
    {
    }

    template <member_function Fn>
    vfunc(T *instance, size_t table_offset, Fn func)
        : vfunc(instance, table_offset, get_call_type(func), get_vfunc_index(instance, table_offset, func))
    {
    }

    template <typename... Args2>
    Ret operator()(Args2 &&...args) const
    {
        return invoker::template call<Ret>(instance_internal_, func_, call_, std::forward<Args2>(args)...);
    }
};

#define VFUNC_T(call__, __call)                           \
    template <typename Ret, typename T, typename... Args> \
    vfunc(T *instance, size_t table_offset, Ret (__call T::*func)(Args...)) -> vfunc<call__, Ret, T, Args...>;

X86_CALL_MEMBER(VFUNC_T);
#undef VFUNC_T

template <typename T>
class vtable : public instance_holder<T>
{
    using instance_holder<T>::instance_;
    using instance_holder<T>::instance_internal_;
    size_t vtable_offset_;

  public:
    using instance_pointer = T *;
    using table_pointer    = void **;

    vtable(instance_pointer instance = nullptr, size_t vtable_offset = 0)
        : instance_holder<T>(instance)
        , vtable_offset_(vtable_offset)
    {
    }

    template <typename From, typename To>
    [[deprecated]] //
    vtable(magic_cast<From, To> val, size_t vtable_offset = 0)
        : instance_holder<T>(val)
        , vtable_offset_(vtable_offset)
    {
    }

    /*vtable &operator=(std::convertible_to<pointer> auto instance)
    {
        instance_ = instance;
        return *this;
    }*/

    operator instance_pointer() const
    {
        return instance_;
    }

    instance_pointer operator->() const
    {
        return instance_;
    }

    operator table_pointer() const
    {
        return static_cast<void ***>(instance_internal_)[vtable_offset_];
    }

    table_pointer get() const
    {
        return static_cast<void ***>(instance_internal_)[vtable_offset_];
    }

    void set(table_pointer pointer) const
    {
        static_cast<void ***>(instance_internal_)[vtable_offset_] = pointer;
    }

  private:
    void *func_simple(size_t index) const
    {
        return static_cast<void ***>(instance_internal_)[vtable_offset_][index];
    }

  public:
    custom_vfunc<vtable_call<T>, T> func(size_t index) const
    {
        return {instance_, vtable_offset_, index};
    }

    template <typename Ret, typename... Args>
    vfunc<vtable_call<T>, Ret, T, Args...> func(size_t index) const
    {
        return {instance_, vtable_offset_, index};
    }

    template <member_function Fn>
    auto func(Fn fn) const
    {
        return vfunc(instance_, vtable_offset_, fn);
    }

    template <typename Ret, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return vfunc_invoker<vtable_call<T>, Args...>::template call<Ret>(
            instance_, func_simple(index), static_cast<Args>(args)...);
    }

    template <_x86_call Call, typename Ret, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return vfunc_invoker<Call, Args...>::template call<Ret>(
            instance_, func_simple(index), static_cast<Args>(args)...);
    }

    /*template <typename Ret = void, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return vfunc<Ret, Args...>(instance_, index)(
            static_cast<std::conditional_t<
                std::is_trivially_copyable_v<Args> ? std::is_pointer_v<Args> || sizeof(Args) <= sizeof(uintptr_t[2])
                                                   : std::is_reference_v<Args>,
                Args,
                std::add_lvalue_reference_t<Args>>>(args)...);
    }*/
};

template <typename T>
struct vtable<T *>
{
    vtable(...) = delete;
};

// template <typename T>
// auto exchange(vtable<T> table, typename vtable<T>::table_pointer ptr) -> decltype(ptr)
//{
//     auto backup = table.get();
//     table.set(ptr);
//     return backup;
// }

template <typename T, typename T2 = void>
auto exchange(vtable<T> table, vtable<T2> other) -> typename vtable<T>::table_pointer
{
    auto backup = table.get();
    table.set(other.get());
    return backup;
}

template <typename From, typename To>
vtable(magic_cast<From, To *>) -> vtable<To>;

struct auto_cast_tag;

template <typename From>
vtable(magic_cast<From, auto_cast_tag>) -> vtable<void>;

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

template <_x86_call Call, typename Ret, typename T, typename To, typename... Args>
magic_cast(vfunc<Call, Ret, T, Args...>, To) -> magic_cast<T *, To>;

// template <typename To>
// magic_cast(vfunc_holder, To) -> magic_cast<void *, To>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd