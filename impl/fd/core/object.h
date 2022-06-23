#pragma once

#include <string>

import fd.one_instance;
import fd.hash;

template <typename T>
auto _Object_type_impl()
{
    if constexpr (std::is_abstract_v<T>)
        return std::type_identity<T*>();
    else
        return std::type_identity<T>();
}

template <typename T>
using _Object_t = typename decltype(_Object_type_impl<T>())::type /*std::conditional_t<std::is_abstract_v<T>,T*,T>*/;

template <class T>
concept _Dereferenceable = std::is_reference_v<decltype(*std::declval<T>())>;

template <typename T, typename V>
decltype(auto) _Extract_obj_t(V&& val)
{
    using val_t = decltype(val);
    using raw_t = std::remove_cvref_t<V>;
    if constexpr (std::constructible_from<T, val_t>)
        return T(std::forward<V>(val));
    else if constexpr (std::constructible_from<T, decltype(&val)>)
        return &val;
    else if constexpr (_Dereferenceable<val_t> && std::is_class_v<raw_t>) // V is one_instance_t
        return _Extract_obj_t<T>(*val);
}

#define FD_OBJECT_IMPL(_OBJ_TYPE_, _OBJ_IDX_, _IMPL_)                                                           \
    template <>                                                                                                  \
    template <>                                                                                                  \
    fd::one_instance_getter<_Object_t<_OBJ_TYPE_>>::one_instance_getter(const std::in_place_index_t<_OBJ_IDX_>) \
        : item_(_Extract_obj_t<_Object_t<_OBJ_TYPE_>>(_IMPL_))                                                   \
    {                                                                                                            \
    }

#define FD_OBJECT_GET(_OBJ_TYPE_, /* object index */...) fd::instance_of<_Object_t<_OBJ_TYPE_> __VA_OPT__(, ) __VA_ARGS__>

#define FD_OBJECT_BIND(_OBJ_TYPE_, _OBJ_IDX_, _TARGET_TYPE_, /* target index */...) FD_OBJECT_IMPL(_OBJ_TYPE_, _OBJ_IDX_, FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__))
#define FD_OBJECT_BIND_NAME(_OBJ_NAME_, _TARGET_NAME_)                              FD_OBJECT_IMPL(decltype(_OBJ_NAME_)::element_type, _OBJ_NAME_, _TARGET_NAME_)
#define FD_OBJECT_BIND_TYPE(_OBJ_NAME_, _TARGET_TYPE_, /* target index */...)       FD_OBJECT_BIND(decltype(_OBJ_NAME_)::element_type, _OBJ_NAME_, _TARGET_TYPE_, __VA_ARGS__)

#define FD_OBJECT(_OBJ_NAME_, _OBJ_TYPE_, /* object index */...) constexpr auto _OBJ_NAME_ = FD_OBJECT_GET(_OBJ_TYPE_, __VA_ARGS__);
#define FD_UNIQUE_OBJECT(_OBJ_NAME_, _OBJ_TYPE_) FD_OBJECT(_OBJ_NAME_, _OBJ_TYPE_, /* fd::calc_hash(#_OBJ_NAME_) */ fd::calc_hash(__FILE__) + __COUNTER__);
/*
example:

FD_OBJECT(name, type, 1337)

FD_OBJECT_BIND(type, 1337/name, real_type, ...)
or
FD_OBJECT_IMPL(type, 1337/name, get_real_type())
*/
