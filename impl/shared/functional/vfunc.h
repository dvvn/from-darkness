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
    static_assert(std::is_member_function_pointer_v<Fn>);
#endif
    constexpr auto call = function_info<Fn>::call;
    auto function       = unsafe_cast<void*>(table_function);
    return get_vfunc<call>(function, instance);
}

inline void* get_vfunc(ptrdiff_t const function_index, void* instance)
{
    return get_vtable(instance)[function_index];
}

struct vfunc_tag
{
};

template <class Object>
class basic_vfunc : public vfunc_tag
{
    void* function_;
    Object* instance_;

  public:
    basic_vfunc(void* function, Object* instance)
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

    Object* instance() const
    {
        return instance_;
    }
};

template <typename Object>
class basic_vfunc<Object const>;

template <call_type Call_T, typename Ret, class Object, typename... Args>
struct vfunc : basic_vfunc<Object>
{
    using function_type = typename member_function<Call_T, Ret, Object, Args...>::type;

    using basic_vfunc<Object>::basic_vfunc;

    vfunc(function_type function, Object* instance)
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    function_type get_full() const
    {
        return unsafe_cast<function_type>(basic_vfunc<Object>::get());
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

template <call_type Call_T, typename Ret, class Object, typename... Args>
struct vfunc<Call_T, Ret, Object, function_args<Args...>> : vfunc<Call_T, Ret, Object, Args...>
{
    using vfunc<Call_T, Ret, Object, Args...>::vfunc;
};

#if 0
template <typename Func, class Object>
vfunc(Func fn, Object instance)->typename function_info<Func>::template rebind<vfunc>;
#else
template <typename Func, class Object, class Info = function_info<Func>>
vfunc(Func fn, Object instance) -> vfunc<Info::call, typename Info::return_type, typename Info::object, typename Info::args>;
#endif

template <call_type Call_T, typename Ret, class Object, typename... Args>
Ret invoke(vfunc<Call_T, Ret, Object, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type Call_T, class Object>
struct unknown_vfunc_args : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc_args(Fn function, Object* instance)
#ifdef _DEBUG
        requires(function_info<Fn>::call == Call_T)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    unknown_vfunc_args(ptrdiff_t const function_index, Object* instance)
        : basic_vfunc<Object>(get_vfunc(function_index, instance), instance)
    {
    }

    template <typename Ret, typename... Args>
    vfunc<Call_T, Ret, Object, Args...> get() const
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    void* operator+(size_t const offset) const
    {
        return static_cast<uint8_t*>(basic_vfunc<Object>::get()) + offset;
    }
};

template <call_type Call_T, class Object, typename... Args>
auto invoke(unknown_vfunc_args<Call_T, Object> func, Args... args) -> member_func_return_type_resolver<Call_T, Object, Args...>
{
    return {func.get(), func.instance(), args...};
}

template <typename Ret, call_type Call_T, class Object, typename... Args>
Ret invoke(unknown_vfunc_args<Call_T, Object> func, Args... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename Ret, class Object, typename... Args>
struct unknown_vfunc_call : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc_call(Fn function, Object* instance)
#ifdef _DEBUG
        requires(std::invocable<Fn, Object*, Args...>)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    template <call_type Call_T>
    auto get(call_type_t<Call_T> = {}) const -> vfunc<Call_T, Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }
};

template <call_type Call_T, typename Ret, class Object, typename... Args>
Ret invoke(unknown_vfunc_call<Ret, Object, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <class Object>
struct unknown_vfunc : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc(Fn function, Object* instance)
#ifdef _DEBUG
        requires(std::convertible_to<Object, typename function_info<Fn>::object>)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    template <call_type Call_T, typename Ret, typename... Args>
    auto get() const -> vfunc<Call_T, Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    template <typename Ret, typename... Args>
    auto get() const -> unknown_vfunc_call<Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    template <call_type Call_T>
    auto get() const -> unknown_vfunc_args<Call_T, Object>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }
};

template <call_type Call_T, typename Ret, class Object, typename... Args>
Ret invoke(unknown_vfunc<Object> func, Args... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type Call_T, class Object, typename... Args>
auto invoke(unknown_vfunc<Object> func, Args... args) -> member_func_return_type_resolver<Call_T, Object, Args...>
{
    return {func.get(), func.instance(), args...};
}
} // namespace fd