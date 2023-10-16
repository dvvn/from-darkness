// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyClangDiagnosticUnusedMacros
#pragma once

#ifdef _DEBUG
#define IMGUI_DEBUG_PARANOID
#else
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_DEBUG_TOOLS
#endif

#define IMGUI_USE_WCHAR32
#define IMGUI_DEFINE_MATH_OPERATORS

#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#define IMGUI_DISABLE_SSE

//---

#ifdef IMGUI_USE_WCHAR32
// ReSharper disable once CppUnusedIncludeDirective
#include <cstdint>
#define ImDrawIdx uint32_t
#endif

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
#endif

#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
// ReSharper disable once CppUnusedIncludeDirective
#include <cmath>
#define ImFabs  std::fabs
#define ImSqrt  std::sqrt
#define ImFmod  std::fmod
#define ImCos   std::cos
#define ImSin   std::sin
#define ImAcos  std::acos
#define ImAtan2 std::atan2
#define ImAtof  std::atof
#define ImCeil  std::ceil
#define ImPow   std::pow
#define ImLog   std::log
#define ImAbs   std::abs

template <typename T>
T ImSign(T x) requires requires { std::signbit(x); }
{
    if (x < static_cast<T>(0))
        return static_cast<T>(-1);
    if (x > static_cast<T>(0))
        return static_cast<T>(1);
    return static_cast<T>(0);
}

template <typename T>
T ImRsqrt(T x) requires requires { ImSqrt(x); }
{
    return static_cast<T>(1) / ImSqrt(x);
}
#endif

#define IM_STRV_CLASS_EXTRA                                                                 \
    template <size_t S>                                                                     \
    constexpr ImStrv(char const(&str)[S])                                                   \
        : Begin(str)                                                                        \
        , End(str + S - 1)                                                                  \
    {                                                                                       \
    }                                                                                       \
    constexpr ImStrv(const char* begin, size_t length)                                      \
        : Begin(begin)                                                                      \
        , End(begin + length)                                                               \
    {                                                                                       \
    }                                                                                       \
    template <class T>                                                                      \
    constexpr ImStrv(T const& rng) requires requires { Begin = rng.data() + rng.length(); } \
        : ImStrv(rng.data(), rng.length())                                                  \
    {                                                                                       \
    }