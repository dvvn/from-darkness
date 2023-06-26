#pragma once

// #include "fd/library_info/native.h"
#include "fd/library_info/tag.h"
#include "fd/vtable.h"

namespace fd
{
template <library_tag Tag>
struct native_return_address_gadget
{
    inline static uintptr_t address;
};

#define FD_NATIVE_INTERFACE(_NAME_)                                      \
                                                                         \
    impl::native_vtable<_NAME_> __vtable;                                \
                                                                         \
  private:                                                               \
    template <size_t Index, typename Ret, typename... Args>              \
    using function = impl::native_function<_NAME_, Index, Ret, Args...>; \
                                                                         \
  public:                                                                \
    auto operator[](auto index) const->decltype(__vtable[index])         \
    {                                                                    \
        return __vtable[index];                                          \
    }

#define FD_BIND_NATIVE_INTERFACE(_MEMBER_, _LIB_)                                       \
    inline namespace native                                                             \
    {                                                                                   \
    struct _MEMBER_;                                                                    \
    }                                                                                   \
    template <>                                                                         \
    struct return_address_gadget<_MEMBER_> final : native_return_address_gadget<#_LIB_> \
    {                                                                                   \
    };

struct native_function_tag
{
};

namespace impl
{
template <class T>
struct native_vtable : basic_vtable<T>, noncopyable
{
#if !defined(_DEBUG)
    static_assert(valid_return_address_gadget<T>);
#endif
    using basic_vtable<T>::basic_vtable;
    using basic_vtable<T>::operator[];

    native_vtable(void *instance)
        : basic_vtable<T>(static_cast<T *>(instance))
    {
    }
};

/**
 * \tparam Index number or \code vfunc_index<X>
 */
template <class T, auto Index, typename Ret, typename... Args>
class native_function :public native_function_tag
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

template <typename I, typename A>
auto native_interface_cast(A address)
{
    static_assert(sizeof(A) == sizeof(void *));

    if constexpr (/*std::constructible_from<I, void *>*/ std::is_union_v<I>)
        return I{reinterpret_cast<void *>(address)};
    else
        return reinterpret_cast<I *>(address);
}
} // namespace impl

// ReSharper disable once CppInconsistentNaming
#define __EXAMPLE                                                     \
    union example                                                     \
    {                                                                 \
        FD_ABSTRACT_INTERFACE(example);                               \
        native_function<1, void, int> func;                           \
        native_function<10, int, named_arg<char, "something">> func1; \
    };                                                                \
    void init()                                                       \
    {                                                                 \
        void **instance;                                              \
        example test(*instance);                                      \
        example *test2 = reinterpret_cast<example *>(instance);       \
    }
#undef __EXAMPLE

} // namespace fd