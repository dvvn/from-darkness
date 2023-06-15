#pragma once

#include "named_arg.h"
#include "vtable.h"

namespace fd
{

template <class T>
struct abstract_interface : vtable<void>, noncopyable
{
#if !defined(_DEBUG)
    static_assert(valid_return_address_gadget<T>);
#endif
    using vtable::vtable;
};

/**
 * \brief for visualization only
 */
template <typename T, named_arg Name>
class named
{
    T object_;

  public:
    named(std::convertible_to<T> auto object)
        : object_(object)
    {
    }

    operator T() const
    {
        (void)Name;
        return object_;
    }
};

template <typename T, named_arg Name>
class named<T &, Name>
{
    T &object_;

  public:
    named(T &object)
        : object_(object)
    {
    }

    operator T &() const
    {
        (void)Name;
        return object_;
    }
};

template <typename T>
struct make_unnamed
{
    using type = T;
};

template <typename T, auto Name>
struct make_unnamed<named<T, Name>>
{
    using type = T;
};

struct abstract_function_tag : noncopyable
{
};

template <auto Index>
concept valid_abstract_function_index = requires() { static_cast<size_t>(Index); };

/**
 * \tparam Index number or \code vfunc_index<X>
 */
template <class T, auto Index, typename Ret, typename... Args>
class abstract_function : public abstract_function_tag
{
    vtable<T> table_;

  public:
    static_assert(valid_abstract_function_index<Index>);

    abstract_function(T *instance)
        : table_(instance)

    {
    }

    auto get() const
    {
        return table_[Index].template get<Ret, typename make_unnamed<Args>::type...>();
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

#if 0
template <typename T>
concept has_abstract_interface = requires(T object) {
    []<typename Q>(abstract_interface<Q> const &) {
    }(object.vtable);
};

template <has_abstract_interface T>
auto get(T const &object, auto index) -> decltype(object.vtable[index])
{
    return object.vtable[index];
}
#endif

#define FD_ABSTRACT_INTERFACE(_NAME_)                                         \
    template <size_t Index, typename Ret, typename... Args>                   \
    using abstract_function = abstract_function<_NAME_, Index, Ret, Args...>; \
                                                                              \
    abstract_interface<_NAME_> vtable;                                        \
                                                                              \
    auto operator[](auto index) const->decltype(_NAME_::vtable[index])        \
    {                                                                         \
        return _NAME_::vtable[index];                                         \
    }

// ReSharper disable once CppInconsistentNaming
#define __EXAMPLE                                                       \
    union example                                                       \
    {                                                                   \
        FD_ABSTRACT_INTERFACE(example);                                 \
        abstract_function<1, void, int> func;                           \
        abstract_function<10, int, named_arg<char, "something">> func1; \
    };                                                                  \
    void init()                                                         \
    {                                                                   \
        void **instance;                                                \
        example test(*instance);                                        \
        example *test2 = reinterpret_cast<example *>(instance);         \
    }
#undef __EXAMPLE

} // namespace fd