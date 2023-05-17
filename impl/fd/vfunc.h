#pragma once

#include "x86_call.h"

#include <concepts>
#include <tuple>
#include <utility>

#undef cdecl

namespace fd
{
template <typename From, typename To>
class magic_cast;

size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, _x86_call call);

template <typename Fn>
void *get_function_pointer(Fn function)
{
    static_assert(sizeof(Fn) == sizeof(void *));

    union
    {
        Fn fn;
        void *fn_ptr;
    };

    fn = function;
    return fn_ptr;
}

template <typename Fn>
size_t get_vfunc_index(void *instance, size_t vtable_offset, Fn function)
{
    /*static_assert(member_function<Fn>);*/
    return get_vfunc_index(instance, vtable_offset, get_function_pointer(function), get_call_type(function));
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

template <_x86_call Call, typename... Args>
struct vfunc_invoker;

struct vfunc_invoker_gap
{
};

#define VFUNC_INVOKER(call__, __call, _call_)                                     \
    template <typename... Args>                                                   \
    struct vfunc_invoker<call__, Args...>                                         \
    {                                                                             \
        template <typename Ret>                                                   \
        static Ret call(void *instance, void *fn, Args... args) noexcept          \
        {                                                                         \
            union                                                                 \
            {                                                                     \
                void *fn1;                                                        \
                Ret (__call vfunc_invoker_gap::*fn2)(Args...);                    \
            };                                                                    \
            fn1 = fn;                                                             \
            return (*static_cast<vfunc_invoker_gap *>(instance).*fn2)((args)...); \
        }                                                                         \
    };

X86_CALL_MEMBER(VFUNC_INVOKER);
#undef VFUNC_INVOKER

template <typename... Args>
struct vfunc_invoker<_x86_call::unknown, Args...>
{
    template <typename Ret>
    static Ret call(void *instance, void *fn, _x86_call info, Args... args) noexcept
    {
#define SELECT_INVOKER(call__, __call, _call_) \
    case call__:                               \
        return vfunc_invoker<call__, Args...>::template call<Ret>(instance, fn, (args)...);

        switch (info)
        {
            X86_CALL_MEMBER(SELECT_INVOKER);
        default:
            std::unreachable();
        }

#undef SELECT_INVOKER
    }

    template <typename Ret, _x86_call Call>
    static Ret call(void *instance, void *fn, call_type_t<Call>, Args... args) noexcept
    {
        return vfunc_invoker<Call, Args...>::template call<Ret>(instance, fn, (args)...);
    }

    template <typename Ret, member_function Fn>
    static Ret call(void *instance, Fn fn, Args... args) noexcept
    {
        return vfunc_invoker<get_call_type(fn), Args...>::template call<Ret>(
            instance, get_function_pointer(fn), (args)...);
    }
};

template <typename T>
constexpr _x86_call vtable_call = _x86_call::thiscall__;

template <typename T>
class instance_holder
{
  protected:
    union
    {
        T *instance_;
        void *instance_void_;
        void ***vtable_;
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
  protected:
    void *func_;

  public:
    vfunc_holder(void *func)
        : func_(func)
    {
    }

    vfunc_holder(void ***table, size_t table_offset, size_t func_index)
        : func_(table[table_offset][func_index])
    {
    }

    vfunc_holder(void *instance, size_t table_offset, size_t func_index)
        : vfunc_holder(static_cast<void ***>(instance), table_offset, func_index)
    {
    }

    template <member_function Fn>
    vfunc_holder(void *instance, size_t table_offset, Fn function)
        : vfunc_holder(static_cast<void ***>(instance), table_offset, get_vfunc_index(instance, table_offset, function))
    {
    }

    void *get() const
    {
        return func_;
    }
};

template <typename T>
struct return_type_t
{
    using value_type = T;
};

template <typename T>
constexpr return_type_t<T> return_type;

using unknown_return_type = return_type_t<nullptr_t>;

template <_x86_call Call, typename Ret, typename T, typename... Args>
class vfunc : public instance_holder<T>, public vfunc_holder
{
    using invoker = vfunc_invoker<Call, Args...>;

    using instance_holder<T>::instance_void_;

  public:
    vfunc(T *instance, size_t table_offset, auto function)
        : instance_holder<T>(instance)
        , vfunc_holder(instance_void_, table_offset, function)
    {
    }

    Ret operator()(Args... args) const
    {
        return invoker::template call<Ret>(instance_void_, func_, (args)...);
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
    operator Ret() const
    {
        static_assert(!std::is_reference_v<Ret>, "Not implemented");
        return fn_(return_type<Ret>);
    }
};

template <_x86_call Call, typename T>
class vfunc<Call, unknown_return_type, T> : public instance_holder<T>, public vfunc_holder
{
    template <typename... Args>
    using invoker = vfunc_invoker<Call, Args...>;

    using instance_holder<T>::instance_void_;

  public:
    vfunc(T *instance, size_t table_offset, auto function)
        : instance_holder<T>(instance)
        , vfunc_holder(instance_void_, table_offset, function)
    {
    }

    template <typename Ret, typename... Args>
    Ret operator()(return_type_t<Ret>, Args... args) const
    {
        return invoker<Args...>::template call<Ret>(instance_void_, func_, args...);
    }

    template <_x86_call Call_1>
    auto operator()(call_type_t<Call_1>, auto...) const = delete;

    template <typename... Args>
    auto operator()(Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Args...>::template call<Ret>(instance_void_, func_, args...);
        });
    }
};

template <typename Ret, typename T, typename... Args>
class vfunc<_x86_call::unknown, Ret, T, Args...> : public instance_holder<T>, public vfunc_holder
{
    using invoker = vfunc_invoker<_x86_call::unknown, Args...>;

    using instance_holder<T>::instance_void_;

  public:
    vfunc(T *instance, size_t table_offset, auto function)
        : instance_holder<T>(instance)
        , vfunc_holder(instance_void_, table_offset, function)
    {
    }

    template <_x86_call Call>
    Ret operator()(call_type_t<Call> call, Args... args) const
    {
        return invoker::template call<Ret>(instance_void_, func_, call, args...);
    }
};

template <typename T>
class vfunc<_x86_call::unknown, unknown_return_type, T> : public instance_holder<T>, public vfunc_holder
{
    template <typename... Args>
    using invoker = vfunc_invoker<_x86_call::unknown, Args...>;

    using instance_holder<T>::instance_void_;

  public:
    vfunc(T *instance, size_t table_offset, auto func_info)
        : instance_holder<T>(instance)
        , vfunc_holder(instance_void_, table_offset, func_info)
    {
    }

    template <_x86_call Call, typename Ret, typename... Args>
    Ret operator()(call_type_t<Call> call, return_type_t<Ret>, Args... args) const
    {
        return invoker<Args...>::template call<Ret>(instance_void_, func_, call, args...);
    }

    template <_x86_call Call, typename... Args>
    auto operator()(call_type_t<Call> call, Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Args...>::template call<Ret>(instance_void_, func_, call, args...);
        });
    }
};

template <typename T>
vfunc(T *instance, size_t table_offset, size_t func_index) -> vfunc<_x86_call::unknown, unknown_return_type, T>;

#define VFUNC_T(call__, __call, call)                     \
    template <typename Ret, typename T, typename... Args> \
    vfunc(T *instance, size_t table_offset, Ret (__call T::*func)(Args...)) -> vfunc<call__, Ret, T, Args...>;

X86_CALL_MEMBER(VFUNC_T);
#undef VFUNC_T

template <typename T>
class vtable : public instance_holder<T>
{
    using instance_holder<T>::instance_;
    using instance_holder<T>::vtable_;

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
    vtable(magic_cast<From, To> val, size_t vtable_offset = 0)
        : instance_holder<T>(static_cast<instance_pointer>(val))
        , vtable_offset_(vtable_offset)
    {
    }

    template <typename From, typename To>
    vtable &operator=(magic_cast<From, To> val)
    {
        instance_holder<T>::operator=(static_cast<T *>(val));
        return *this;
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
        return vtable_[vtable_offset_];
    }

    table_pointer get() const
    {
        return vtable_[vtable_offset_];
    }

    void set(table_pointer pointer) const
    {
        vtable_[vtable_offset_] = pointer;
    }

  private:
    void *func_simple(size_t index) const
    {
        return vtable_[vtable_offset_][index];
    }

  public:
    template <std::integral I>
    vfunc<vtable_call<T>, unknown_return_type, T> operator[](I index) const
    {
        return {instance_, vtable_offset_, index};
    }

    template <member_function Fn>
    auto operator[](Fn fn) const
    {
        return vfunc(instance_, vtable_offset_, fn);
    }
};

template <typename T>
struct vtable<T *>;

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

template <_x86_call Call, typename Ret, typename T, typename To, typename... Args>
magic_cast(vfunc<Call, Ret, T, Args...>, To) -> magic_cast<T *, To>;

// template <typename To>
// magic_cast(vfunc_holder, To) -> magic_cast<void *, To>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd