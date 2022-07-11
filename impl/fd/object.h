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

#define FD_OBJECT_IMPL(_OBJ_TYPE_, _OBJ_IDX_, _IMPL_)                                                         \
    template <>                                                                                               \
    template <>                                                                                               \
    fd::instance_of_getter<_Object_t<_OBJ_TYPE_>>::instance_of_getter(const std::in_place_index_t<_OBJ_IDX_>) \
    {                                                                                                         \
        _Construct(_IMPL_);                                                                                   \
    }

#define FD_OBJECT_GET(_OBJ_TYPE_, /* object index */...) fd::instance_of<_Object_t<_OBJ_TYPE_> __VA_OPT__(, ) __VA_ARGS__>

#define FD_OBJECT_ATTACH(_OBJ_TYPE_, _TARGET_TYPE_, ...) FD_OBJECT_IMPL(_OBJ_TYPE_, 0, FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__))

#define FD_OBJECT_BIND(_OBJ_NAME_, _TARGET_)                FD_OBJECT_IMPL(decltype(_OBJ_NAME_)::value_type, _OBJ_NAME_, _TARGET_)
#define FD_OBJECT_BIND_TYPE(_OBJ_NAME_, _TARGET_TYPE_, ...) FD_OBJECT_IMPL(decltype(_OBJ_NAME_)::value_type, _OBJ_NAME_, FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__))

#define FD_UNIQUE_INDEX (/* fd::calc_hash(__FILE__) */ fd::unique_hash() + __COUNTER__)

#define FD_OBJECT(_OBJ_NAME_, _OBJ_TYPE_, ...) constexpr auto _OBJ_NAME_ = FD_OBJECT_GET(_OBJ_TYPE_, __VA_ARGS__);
