#pragma once

#include <type_traits>

import fd.one_instance;
import fd.hash;

template <typename T>
auto _Object_type_impl()
{
    constexpr std::type_identity<T*> ptr_t;
    constexpr std::type_identity<T> direct_t;

    /*  if constexpr (!std::is_default_constructible_v<T>) // incomplete class
         return ptr_t;
     else  */
    if constexpr (std::is_abstract_v<T>)
        return ptr_t;
    else
        return direct_t;
}

#if 0
template <typename T>
using _Object_t = typename decltype(_Object_type_impl<T>())::type /*std::conditional_t<std::is_abstract_v<T>,T*,T>*/;

#define FD_OBJECT_IMPL_HEAD(_OBJ_TYPE_, /* object index */...) \
    template <>                                                \
    template <>                                                \
    fd::instance_of_getter<_Object_t<_OBJ_TYPE_>>::instance_of_getter(const fd::_Object_index<__VA_ARGS__>)

#define FD_OBJECT_IMPL_HEAD_BIND(_OBJ_NAME_)                          \
    FD_OBJECT_IMPL_HEAD(decltype(_OBJ_NAME_)::value_type, _OBJ_NAME_) \
    //

#define FD_OBJECT_ATTACH_EX(_OBJ_TYPE_, _IMPL_, ...) \
    FD_OBJECT_IMPL_HEAD(_OBJ_TYPE_, __VA_ARGS__)     \
    {                                                \
        _Construct(_IMPL_);                          \
    }
#endif

#define FD_OBJECT_GET(_OBJ_TYPE_, ...) fd::instance_of<_OBJ_TYPE_ __VA_OPT__(, ) __VA_ARGS__>

// #define FD_OBJECT_BIND(_OBJ_NAME_, _TARGET_)                FD_OBJECT_ATTACH_EX(decltype(_OBJ_NAME_)::value_type, _TARGET_, _OBJ_NAME_)
// #define FD_OBJECT_BIND_TYPE(_OBJ_NAME_, _TARGET_TYPE_, ...) FD_OBJECT_ATTACH_EX(decltype(_OBJ_NAME_)::value_type, FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__), _OBJ_NAME_)
// #define FD_OBJECT_ATTACH(_OBJ_TYPE_, _TARGET_TYPE_, ...) FD_OBJECT_ATTACH_EX(_OBJ_TYPE_, FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__))

#ifndef _CONCAT
#define _CONCATX(x, y) x##y
#define _CONCAT(x, y)  _CONCATX(x, y)
#endif

#define _DUMMY_VAR(_NAME_) _CONCAT(_NAME_, __LINE__)
#define _DUMMY_OBJ_VAR     static const auto _DUMMY_VAR(_lazy_obj_)

#define FD_OBJECT_BIND(_OBJ_NAME_, _TARGET_)        \
    _DUMMY_OBJ_VAR = _OBJ_NAME_.construct_lazy([] { \
        return &_TARGET_;                           \
    });

#define FD_OBJECT_BIND_EX(_OBJ_NAME_, ...)          \
    _DUMMY_OBJ_VAR = _OBJ_NAME_.construct_lazy([] { \
        return __VA_ARGS__;                         \
    });

#define FD_OBJECT_BIND_TYPE(_OBJ_NAME_, _TARGET_TYPE_, ...) \
    _DUMMY_OBJ_VAR = _OBJ_NAME_.construct_lazy([] {         \
        return &FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__);  \
    });

#define FD_OBJECT_ATTACH(_OBJ_TYPE_, _TARGET_TYPE_, ...)           \
    _DUMMY_OBJ_VAR = FD_OBJECT_GET(_OBJ_TYPE_).construct_lazy([] { \
        return &FD_OBJECT_GET(_TARGET_TYPE_, __VA_ARGS__);         \
    });

#define FD_OBJECT_ATTACH_EX(_OBJ_TYPE_, ...)                       \
    _DUMMY_OBJ_VAR = FD_OBJECT_GET(_OBJ_TYPE_).construct_lazy([] { \
        return __VA_ARGS__;                                        \
    });

#define FD_OBJECT_ATTACH_MANUAL(_OBJ_TYPE_, ...)                                             \
    static decltype(FD_OBJECT_GET(_OBJ_TYPE_))::value_type _DUMMY_VAR(_lazy_obj_impl_)();    \
    _DUMMY_OBJ_VAR = FD_OBJECT_GET(_OBJ_TYPE_).construct_lazy(&_DUMMY_VAR(_lazy_obj_impl_)); \
    decltype(FD_OBJECT_GET(_OBJ_TYPE_))::value_type _DUMMY_VAR(_lazy_obj_impl_)()

#define FD_UNIQUE_INDEX (fd::unique_hash(__FILE__) + __COUNTER__)

#define FD_OBJECT(_OBJ_NAME_, _OBJ_TYPE_, ...) constexpr auto _OBJ_NAME_ = FD_OBJECT_GET(_OBJ_TYPE_, __VA_ARGS__);
