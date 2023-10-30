#pragma once

#include "functional/call_traits.h"

#include <concepts>

namespace fd
{
size_t get_vfunc_index(call_type call, void* function);

#define GET_VFUNC_IDX(call__, __call, _call_) \
    /**/                                      \
    size_t get_vfunc_index(call_type_t<call__> call, void* function);

X86_CALL_MEMBER(GET_VFUNC_IDX);
#undef GET_VFUNC_IDX

inline void** get_vtable(void* instance)
{
    return *static_cast<void***>(instance);
}

// template <typename T>
// void **get_vtable(T *instance)
//{
//     return get_vtable(static_cast<void *>( (instance)));
// }

inline void* get_vfunc(call_type const call, void* table_function, void* instance)
{
    auto const function_index = get_vfunc_index(call, table_function);
    return get_vtable(instance)[function_index];
}

template <call_type Call_T>
void* get_vfunc(void* table_function, void* instance)
{
    auto const function_index = get_vfunc_index(call_type_v<Call_T>, table_function);
    return get_vtable(instance)[function_index];
}

template <typename Fn>
void* get_vfunc(Fn table_function, void* instance)
{
#ifdef _DEBUG
    static_assert(member_function<Fn>);
#endif
    constexpr auto call = function_info<Fn>::call;
    auto function       = unsafe_cast<void*>(table_function);
    return get_vfunc<call>(function, instance);
}

inline void* get_vfunc(ptrdiff_t const function_index, void* instance)
{
    return get_vtable(instance)[function_index];
}

template <typename T>
class basic_vfunc
{
    void* function_;
    T* instance_;

  public:
    basic_vfunc(void* function, T* instance)
        : function_(function)
        , instance_(instance)
    {
    }

    void* get() const
    {
        return function_;
    }

    operator void*() const
    {
        return function_;
    }

    T* instance() const
    {
        return instance_;
    }
};

template <typename T>
class basic_vfunc<T const>;

template <call_type Call_T, typename Ret, typename T, typename... Args>
struct vfunc : basic_vfunc<T>
{
    using basic_vfunc<T>::basic_vfunc;

    vfunc(member_func_type<Call_T, Ret, T, Args...> function, T* instance)
        : basic_vfunc<T>(get_vfunc(function, instance), instance)
    {
    }

    /*Ret operator()(Args... args) const
    {
        member_func_invoker<Call_T, Ret, T, Args...> invoker;
        return invoker(function_, instance_, args...);
    }*/
};

template <call_type Call_T, typename Ret, typename... Args>
struct vfunc<Call_T, Ret, void, Args...> : basic_vfunc<void>
{
    using basic_vfunc::basic_vfunc;
};

#define VFUNC_CONSTRUCT(call__, __call, _call_)                                             \
    template <typename Ret, typename T, std::convertible_to<T*> Instance, typename... Args> \
    vfunc(Ret (__call T::*func)(Args...), Instance function) -> vfunc<call__, Ret, T, Args...>;

X86_CALL_MEMBER(VFUNC_CONSTRUCT);
#undef VFUNC_CONSTRUCT

template <call_type Call_T, typename Ret, typename T, typename... Args>
Ret invoke(vfunc<Call_T, Ret, T, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type Call_T, typename T>
struct unknown_vfunc_args : basic_vfunc<T>
{
    template <typename Fn>
    unknown_vfunc_args(Fn function, T* instance)
#ifdef _DEBUG
        requires(function_info<Fn>::call == Call_T)
#endif
        : basic_vfunc<T>(get_vfunc(function, instance), instance)
    {
    }

    unknown_vfunc_args(ptrdiff_t const function_index, T* instance)
        : basic_vfunc<T>(get_vfunc(function_index, instance), instance)
    {
    }

    template <typename Ret, typename... Args>
    vfunc<Call_T, Ret, T, Args...> get() const
    {
        return {basic_vfunc<T>::get(), basic_vfunc<T>::instance()};
    }

    void* operator+(size_t const offset) const
    {
        return static_cast<uint8_t*>(basic_vfunc<T>::get()) + offset;
    }
};

template <call_type Call_T, typename T, typename... Args>
auto invoke(unknown_vfunc_args<Call_T, T> func, Args... args) -> member_func_return_type_resolver<Call_T, T, Args...>
{
    return {func.get(), func.instance(), args...};
}

template <typename Ret, call_type Call_T, typename T, typename... Args>
Ret invoke(unknown_vfunc_args<Call_T, T> func, Args... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename Ret, typename T, typename... Args>
struct unknown_vfunc_call : basic_vfunc<T>
{
    template <typename Fn>
    unknown_vfunc_call(Fn function, T* instance)
#ifdef _DEBUG
        requires(std::invocable<Fn, T*, Args...>)
#endif
        : basic_vfunc<T>(get_vfunc(function, instance), instance)
    {
    }

    template <call_type Call_T>
    vfunc<Call_T, Ret, T, Args...> get(call_type_t<Call_T> = {}) const
    {
        return {basic_vfunc<T>::get(), basic_vfunc<T>::instance()};
    }
};

template <call_type Call_T, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc_call<Ret, T, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename T>
struct unknown_vfunc : basic_vfunc<T>
{
    template <typename Fn>
    unknown_vfunc(Fn function, T* instance)
#ifdef _DEBUG
        requires(std::same_as<typename function_info<Fn>::self_type, T>)
#endif
        : basic_vfunc<T>(get_vfunc(function, instance), instance)
    {
    }

    template <call_type Call_T, typename Ret, typename... Args>
    vfunc<Call_T, Ret, T, Args...> get() const
    {
        return {basic_vfunc<T>::get(), basic_vfunc<T>::instance()};
    }

    template <typename Ret, typename... Args>
    unknown_vfunc_call<Ret, T, Args...> get() const
    {
        return {basic_vfunc<T>::get(), basic_vfunc<T>::instance()};
    }

    template <call_type Call_T>
    unknown_vfunc_args<Call_T, T> get() const
    {
        return {basic_vfunc<T>::get(), basic_vfunc<T>::instance()};
    }
};

template <call_type Call_T, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc<T> func, Args... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type Call_T, typename T, typename... Args>
auto invoke(unknown_vfunc<T> func, Args... args) -> member_func_return_type_resolver<Call_T, T, Args...>
{
    return {func.get(), func.instance(), args...};
}
} // namespace fd