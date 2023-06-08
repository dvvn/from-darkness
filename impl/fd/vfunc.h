#pragma once

#include "call_type.h"
#include "core.h"

#include <concepts>
#include <utility>

#undef cdecl

namespace fd
{
size_t get_vfunc_index(call_type_t call, void *function, void *instance, size_t vtable_offset);

template <typename T>
void ***get_vtable(T *instance)
{
    return static_cast<void ***>(/*remove_const*/ (instance));
}

inline void ***get_vtable(void *instance)
{
    return static_cast<void ***>(instance);
}

inline void *get_vfunc(call_type_t call, void *table_function, void *instance, size_t vtable_offset)
{
    auto function_index = get_vfunc_index(call, table_function, instance, vtable_offset);
    return get_vtable(instance)[vtable_offset][function_index];
}

template <typename Fn>
void *get_vfunc(Fn table_function, void *instance, size_t vtable_offset)
{
    return get_vfunc(get_call_type(table_function), get_function_pointer(table_function), instance, vtable_offset);
}

inline void *get_vfunc(size_t function_index, void *instance, size_t vtable_offset)
{
    return get_vtable(instance)[vtable_offset][function_index];
}

template <typename T>
constexpr call_type_t vtable_call = call_type_t::thiscall_;

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

template <call_type_t Call, typename Ret, typename T, typename... Args>
class vfunc
{
    using builder = member_func_builder<Call, Ret, T, Args...>;

    VFUNC_BASE;

    vfunc(void *function, T *instance)
        : function_(function)
        , instance_(instance)
    {
    }

    vfunc(typename builder::type function, T *instance, size_t table_offset)
        : function_(
              get_vfunc(builder::call_type, get_function_pointer(function), /*remove_const*/ (instance), table_offset))
        , instance_(instance)
    {
    }

    Ret operator()(Args... args) const
    {
        return builder::invoke(function_, instance_, (args)...);
    }
};

template <call_type_t Call, typename Ret, typename T, typename... Args>
Ret invoke(vfunc<Call, Ret, T, Args...> func, Args... args)
{
    using builder = member_func_builder<Call, Ret, T, Args...>;
    return builder::invoke(func.get(), func.instance(), (args)...);
}

template <call_type_t Call, typename T>
class unknown_vfunc_args
{
    VFUNC_BASE;

    template <typename Ret, typename... Args>
    vfunc<Call, Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }

    template <same_call_type<Call> Fn>
    unknown_vfunc_args(Fn function, T *instance, size_t table_offset)
        : function_(get_vfunc(function, /*remove_const*/ (instance), table_offset))
        , instance_(instance)
    {
    }

    unknown_vfunc_args(size_t function_index, T *instance, size_t table_offset)
        : function_(get_vfunc(function_index, /*remove_const*/ (instance), table_offset))
        , instance_(instance)
    {
    }
};

template <call_type_t Call, typename T, typename... Args>
auto invoke(unknown_vfunc_args<Call, T> func, Args... args) -> member_func_return_type_resolver<Call, T, Args...>
{
    return {func.get(), func.instance(), args...};
}

template <typename Ret, call_type_t Call, typename T, typename... Args>
Ret invoke(unknown_vfunc_args<Call, T> func, Args... args)
{
    using builder = member_func_builder<Call, Ret, T, Args...>;
    return builder::invoke(func.get(), func.instance(), args...);
}

template <typename Ret, typename T, typename... Args>
class unknown_vfunc_call
{
    VFUNC_BASE;

    template <typename Fn>
    unknown_vfunc_call(T *instance, size_t table_offset, Fn function)
        : function_(get_vfunc(/*remove_const*/ (instance), table_offset, function))
        , instance_(instance)
    {
        static_assert(std::invocable<Fn, T *, Args...>);
    }

    template <call_type_t Call>
    vfunc<Call, Ret, T, Args...> get(call_type_holder<Call> = {}) const
    {
        return {instance_, function_};
    }
};

template <call_type_t Call, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc_call<Ret, T, Args...> func, Args... args)
{
    using builder = member_func_builder<Call, Ret, T, Args...>;
    return builder::invoke(func.get(), func.instance(), args...);
}

template <typename T>
class unknown_vfunc
{
    VFUNC_BASE;

    template <call_type_t Call, typename Ret, typename... Args>
    vfunc<Call, Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }

    template <typename Ret, typename... Args>
    unknown_vfunc_call<Ret, T, Args...> get() const
    {
        return {function_, instance_};
    }

    template <call_type_t Call>
    unknown_vfunc_args<Call, T> get() const
    {
        return {function_, instance_};
    }

    template <typename Fn>
    unknown_vfunc(T *instance, size_t table_offset, Fn function)
        : function_(get_vfunc(/*remove_const*/ (instance), table_offset, function))
        , instance_(instance)
    {
    }
};

#undef VFUNC_BASE

template <call_type_t Call, typename Ret, typename T, typename... Args>
Ret invoke(unknown_vfunc<T> func, Args... args)
{
    using builder = member_func_builder<Call, Ret, T, Args...>;
    return builder::invoke(func.get(), func.instance(), args...);
}

template <call_type_t Call, typename T, typename... Args>
auto invoke(unknown_vfunc<T> func, Args... args) -> member_func_return_type_resolver<Call, T, Args...>
{
    return {func.get(), func.instance(), args...};
}

struct vtable_backup : noncopyable
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

  public:
    basic_vtable(instance_pointer instance = nullptr, size_t vtable_offset = 0)
        : instance_(instance)
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
        vtable_[vtable_offset_] = pointer;
    }

    vtable_backup replace(table_pointer pointer)
    {
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

    unknown_vfunc_args<vtable_call<T>, T> operator[](size_t index) const
    {
        return {index, instance_, vtable_offset_};
    }
};

template <typename T>
struct vtable : basic_vtable<T>
{
    using basic_vtable<T>::basic_vtable;
    using basic_vtable<T>::operator[];

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename... Args>                                       \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, basic_vtable<T>::instance_, basic_vtable<T>::vtable_offset_}; \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <>
struct vtable<void> : basic_vtable<void>
{
    using basic_vtable::basic_vtable;
    using basic_vtable::operator[];

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename T, typename... Args>                           \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, instance_, vtable_offset_};                                   \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <typename T>
vtable(T *, size_t = 0) -> vtable<T>;
} // namespace fd