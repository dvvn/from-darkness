#pragma once

#include <utility>

import nstd.one_instance;

template <typename T>
auto _Object_type_impl()
{
    if constexpr (std::is_abstract_v<T>)
        return std::type_identity<T*>();
    else
        return std::type_identity<T>();
}

template <typename T>
using _Object_type = typename decltype(_Object_type_impl<T>())::type;

template <typename T, typename V>
auto _Correct_result(V val)
{
    if constexpr (std::constructible_from<T, V>)
        return std::move(val);
    else if constexpr (std::is_pointer_v<T>)
        return &val;
}

#define CHEAT_OBJECT_IMPL(_OBJ_TYPE_, _OBJ_IDX_, _IMPL_)                                                             \
    template <>                                                                                                      \
    template <>                                                                                                      \
    nstd::one_instance_getter<_Object_type<_OBJ_TYPE_>>::one_instance_getter(const std::in_place_index_t<_OBJ_IDX_>) \
        : item_(_Correct_result<_Object_type<_OBJ_TYPE_>>(_IMPL_))                                                   \
    {                                                                                                                \
    }

#define CHEAT_OBJECT_GET(_OBJ_TYPE_, /* _OBJ_IDX_ */...) nstd::instance_of<_Object_type<_OBJ_TYPE_>, ##__VA_ARGS__>

#define CHEAT_OBJECT_BIND(_OBJ_TYPE_, _OBJ_IDX_, _TARGET_TYPE_, /* _TARGET_IDX_ */...) CHEAT_OBJECT_IMPL(_OBJ_TYPE_, _OBJ_IDX_, CHEAT_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__))
#define CHEAT_OBJECT_BIND_SIMPLE(_OBJ_TYPE_, _TARGET_TYPE_) CHEAT_OBJECT_BIND(_OBJ_TYPE_, 0, _TARGET_TYPE_, 0)

#define CHEAT_OBJECT(_OBJ_NAME_, _OBJ_TYPE_, /* _OBJ_IDX_ */...) constexpr auto _OBJ_NAME_ = CHEAT_OBJECT_GET(_OBJ_TYPE_, __VA_ARGS__);

/*
example:

CHEAT_OBJECT(name, type, 1337)

CHEAT_OBJECT_BIND(type, 1337/name, real_type, ...)
or
CHEAT_OBJECT_IMPL(type, 1337/name, get_real_type())
*/
