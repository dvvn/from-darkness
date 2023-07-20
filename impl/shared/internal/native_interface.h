#pragma once

// #include "fd/library_info/native.h"
#include "functional/vtable.h"
#ifdef FD_SPOOF_RETURN_ADDRESS
#include "library_info/tag.h"
#endif

namespace fd
{

struct native_function_tag
{
};

namespace detail
{

#if defined(_DEBUG) || !defined(FD_SPOOF_RETURN_ADDRESS)
template <class T>
using native_vtable = vtable<T>;
#else
template <class T>
struct native_vtable : vtable<T>
{
    static_assert(valid_return_address_gadget<T>);
    using vtable<T>::vtable;
};
#endif

/**
 * \tparam Index number or \code vfunc_index<X>
 */
template <class T, auto Index, typename Ret, typename... Args>
class native_function : public native_function_tag
{
    vtable<T> table_;

  public:
    native_function(T *instance)
        : table_(instance)

    {
    }

    auto get() const
    {
        return table_[Index].template get<Ret, Args...>();
    }

    Ret operator()(Args... args) const
    {
        return invoke(get(), args...);
    }

    static constexpr size_t index()
    {
        return Index;
    }
};
} // namespace detail

template <typename T>
T *construct_at(T *obj, void *ptr) requires requires { &T::__vtable; }
{
    remove_const(obj)->__vtable = ptr;
    return obj;
}

#define FD_NATIVE_INTERFACE_FN(_NAME_)                      \
    template <size_t Index, typename Ret, typename... Args> \
    using function = detail::native_function<_NAME_, Index, Ret, Args...>;

#define FD_NATIVE_INTERFACE(_NAME_)                              \
    detail::native_vtable<_NAME_> __vtable;                      \
    FD_NATIVE_INTERFACE_FN(_NAME_);                              \
    auto operator[](auto index) const->decltype(__vtable[index]) \
    {                                                            \
        return __vtable[index];                                  \
    }

#ifdef FD_SPOOF_RETURN_ADDRESS
template <library_tag Tag>
struct native_return_address_gadget
{
    inline static uintptr_t address;
};
#ifdef __RESHARPER__
#define FD_BIND_NATIVE_INTERFACE_GADGET(_MEMBER_, _LIB_) \
    template <>                                          \
    struct return_address_gadget<_MEMBER_> final         \
    {                                                    \
    };
#else
#define FD_BIND_NATIVE_INTERFACE_GADGET(_MEMBER_, _LIB_)                                \
    template <>                                                                         \
    struct return_address_gadget<_MEMBER_> final : native_return_address_gadget<#_LIB_> \
    {                                                                                   \
    };
#endif
#define FD_BIND_NATIVE_INTERFACE(_MEMBER_, _LIB_) \
    inline namespace native                       \
    {                                             \
    struct _MEMBER_;                              \
    }                                             \
    FD_BIND_NATIVE_INTERFACE_GADGET(_MEMBER_, _LIB_);
#else
#define FD_BIND_NATIVE_INTERFACE(_MEMBER_, _LIB_) \
    inline namespace native                       \
    {                                             \
    struct _MEMBER_;                              \
    }
#endif
} // namespace fd