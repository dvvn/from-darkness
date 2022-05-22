#pragma once

#include <utility>

import nstd.one_instance;

template <typename T>
auto _Object_type_impl() noexcept
{
    if constexpr (std::is_abstract_v<T>)
        return std::type_identity<T*>();
    else
        return std::type_identity<T>();
}

template <typename T>
using _Object_type = typename decltype(_Object_type_impl<T>())::type;

template <size_t I = 0>
constexpr size_t _Object_index = I;

template <size_t I = 0>
using _Object_index_t = std::in_place_index_t<I>;

template <typename T, typename V>
auto _Correct_result(V val)
{
    if constexpr (std::constructible_from<T, V>)
        return std::move(val);
    else if constexpr (std::is_pointer_v<T>)
        return &val;
}

#define CHEAT_OBJECT_IMPL(_TYPE_, _IMPL_, ...)                                                                                                                      \
    template <>                                                                                                                                                     \
    template <>                                                                                                                                                     \
    nstd::one_instance_getter<_Object_type<_TYPE_>>::one_instance_getter(const _Object_index_t<__VA_ARGS__>) : item_(_Correct_result<_Object_type<_TYPE_>>(_IMPL_)) \
    {                                                                                                                                                               \
    }

#define CHEAT_OBJECT(_NAME_, _TYPE_, ...) constexpr auto _NAME_ = nstd::instance_of<_Object_type<_TYPE_>, _Object_index<__VA_ARGS__>>;
