#pragma once

#include "call_type.h"
#include "core.h"

#include <concepts>

namespace fd
{
size_t get_vfunc_index(call_type_t call, void *function, void *instance);

#define GET_VFUNC_IDX(call__, __call, _call_) \
    size_t get_vfunc_index(call_type_holder<call__> call, void *function, void *instance);

X86_CALL_MEMBER(GET_VFUNC_IDX);
#undef GET_VFUNC_IDX

inline void **get_vtable(void *instance)
{
    return *static_cast<void ***>(instance);
}

inline void **&get_vtable_ref(void *instance)
{
    return *static_cast<void ***>(instance);
}

// template <typename T>
// void **get_vtable(T *instance)
//{
//     return get_vtable(static_cast<void *>( (instance)));
// }

inline void *get_vfunc(call_type_t call, void *table_function, void *instance)
{
    auto function_index = get_vfunc_index(call, table_function, instance);
    return get_vtable(instance)[function_index];
}

template <call_type_t Call_T>
void *get_vfunc(void *table_function, void *instance)
{
    auto function_index = get_vfunc_index(call_type_holder<Call_T>(), table_function, instance);
    return get_vtable(instance)[function_index];
}

template <typename Fn>
void *get_vfunc(Fn table_function, void *instance)
{
    static_assert(member_function<Fn>);

    constexpr auto call = function_info<Fn>::call_type;
    auto function       = get_function_pointer(table_function);
    return get_vfunc<call>(function, instance);
}

inline void *get_vfunc(size_t function_index, void *instance)
{
    return get_vtable(instance)[function_index];
}

#define VFUNC_BASE          \
    void *function_;        \
    T *instance_;           \
                            \
  public:                   \
    void *get() const       \
    {                       \
        return function_;   \
    }                       \
    operator void *() const \
    {                       \
        return function_;   \
    }                       \
    T *instance() const     \
    {                       \
        return instance_;   \
    }

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
class vfunc
{
    VFUNC_BASE;

    vfunc(void *function, T *instance)
        : function_(function)
        , instance_(instance)
    {
    }

    vfunc(member_func_type<Call_T, Ret, T, Args...> function, T *instance)
        : function_(get_vfunc(function, instance))
        , instance_(instance)
    {
    }

    /*Ret operator()(Args... args) const
    {
        member_func_invoker<Call_T, Ret, T, Args...> invoker;
        return invoker(function_, instance_, args...);
    }*/
};

template <call_type_t Call_T, typename Ret, typename... Args>
class vfunc<Call_T, Ret, void, Args...>
{
    // ReSharper disable once CppInconsistentNaming
    using T = void;

    VFUNC_BASE;

    vfunc(void *function, T *instance)
        : function_(function)
        , instance_(instance)
    {
    }
};

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
Ret invoke(vfunc<Call_T, Ret, T, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type_t Call_T, typename T>
class unknown_vfunc_args
{
    VFUNC_BASE;

    template <typename Fn>
    unknown_vfunc_args(Fn function, T *instance)
        : function_(get_vfunc(function, instance))
        , instance_(instance)
    {
        static_assert(function_info<Fn>::call_type == Call_T);
    }

    unknown_vfunc_args(size_t function_index, T *instance)
        : function_(get_vfunc(function_index, instance))
        , instance_(instance)
    {
    }

    template <typename Ret, typename... Args>
    vfunc<Call_T, Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }
};

template <call_type_t Call_T, typename T, typename... Args>
auto invoke(unknown_vfunc_args<Call_T, T> func, Args... args) -> member_func_return_type_resolver<Call_T, T, Args...>
{
    return {func.get(), func.instance(), args...};
}

template <typename Ret, call_type_t Call_T, typename T, typename... Args>
Ret invoke(unknown_vfunc_args<Call_T, T> func, Args... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename Ret, typename T, typename... Args>
class unknown_vfunc_call
{
    VFUNC_BASE;

    template <typename Fn>
    unknown_vfunc_call(Fn function, T *instance)
        : function_(get_vfunc(function, instance))
        , instance_(instance)
    {
        static_assert(std::invocable<Fn, T *, Args...>);
    }

    template <call_type_t Call_T>
    vfunc<Call_T, Ret, T, Args...> get(call_type_holder<Call_T> = {}) const
    {
        return {instance_, function_};
    }
};

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc_call<Ret, T, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename T>
class unknown_vfunc
{
    VFUNC_BASE;

    template <typename Fn>
    unknown_vfunc(Fn function, T *instance)
        : function_(get_vfunc(function, instance))
        , instance_(instance)
    {
        static_assert(std::same_as<typename function_info<Fn>::self_type, T>);
    }

    template <call_type_t Call_T, typename Ret, typename... Args>
    vfunc<Call_T, Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }

    template <typename Ret, typename... Args>
    unknown_vfunc_call<Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }

    template <call_type_t Call_T>
    unknown_vfunc_args<Call_T, T> get() const
    {
        return {function_, instance_};
    }
};

#undef VFUNC_BASE

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc<T> func, Args... args)
{
    member_func_invoker<Call_T, Ret, T, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <call_type_t Call_T, typename T, typename... Args>
auto invoke(unknown_vfunc<T> func, Args... args) -> member_func_return_type_resolver<Call_T, T, Args...>
{
    return {func.get(), func.instance(), args...};
}
} // namespace fd