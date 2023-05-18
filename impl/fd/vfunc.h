#pragma once

#include "x86_call.h"

#include <concepts>
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
    using invoker = member_func_invoker<Call, Ret, Args...>;

    using instance_holder<T>::instance_void_;

  public:
    vfunc(T *instance, size_t table_offset, auto function)
        : instance_holder<T>(instance)
        , vfunc_holder(instance_void_, table_offset, function)
    {
    }

    Ret operator()(Args... args) const
    {
        return invoker::call(instance_void_, func_, (args)...);
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
    template <typename Ret, typename... Args>
    using invoker = member_func_invoker<Call, Ret, Args...>;

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
        return invoker<Ret, Args...>::call(instance_void_, func_, args...);
    }

    template <_x86_call Call_1>
    auto operator()(call_type_t<Call_1>, auto...) const = delete;

    template <typename... Args>
    auto operator()(Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Ret, Args...>::call(instance_void_, func_, args...);
        });
    }
};

template <typename Ret, typename T, typename... Args>
class vfunc<_x86_call::unknown, Ret, T, Args...> : public instance_holder<T>, public vfunc_holder
{
    using invoker = member_func_invoker<_x86_call::unknown, Ret, Args...>;

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
        return invoker::call(instance_void_, func_, call, args...);
    }
};

template <typename T>
class vfunc<_x86_call::unknown, unknown_return_type, T> : public instance_holder<T>, public vfunc_holder
{
    template <typename Ret, typename... Args>
    using invoker = member_func_invoker<_x86_call::unknown, Args...>;

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
        return invoker<Ret, Args...>::call(instance_void_, func_, call, args...);
    }

    template <_x86_call Call, typename... Args>
    auto operator()(call_type_t<Call> call, Args... args) const
    {
        return vfunc_wrapped_invoker([=]<typename Ret>(return_type_t<Ret>) -> Ret {
            return invoker<Ret, Args...>::call(instance_void_, func_, call, args...);
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

    /*template <typename From, typename To>
    vtable &operator=(magic_cast<From, To> val)
    {
        instance_holder<T>::operator=(static_cast<T *>(val));
        return *this;
    }*/

    vtable &operator=(instance_pointer instance)
    {
        instance_ = instance;
        return *this;
    }

    operator instance_pointer() const
    {
        return instance_;
    }

    /*instance_pointer operator->() const
    {
        return instance_;
    }*/

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

    table_pointer replace(table_pointer pointer) const
    {
        return std::exchange(vtable_[vtable_offset_], pointer);
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