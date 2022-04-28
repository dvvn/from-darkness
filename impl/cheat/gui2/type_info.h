#pragma once

#include <typeinfo>

#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define RTTI_ENABLED
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define RTTI_ENABLED
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define RTTI_ENABLED
#endif
#endif

#ifndef RTTI_ENABLED
#include <string_view> //for nstd.type_name
#endif

#if defined(__cpp_lib_constexpr_typeinfo) || !defined(RTTI_ENABLED)
#define TYPE_INFO_CONSTEXPR constexpr
#else
#define TYPE_INFO_CONSTEXPR
#endif

//#if __cplusplus >= 202002L
//#define TYPE_INFO_CONSTEXPR_VIRTUAL TYPE_INFO_CONSTEXPR
//#else
//#define TYPE_INFO_CONSTEXPR_VIRTUAL
//#endif

using type_info_t =
#ifdef RTTI_ENABLED
const std::type_info&
#else //__cpp_lib_constexpr_string_view
const std::string_view
#endif
;