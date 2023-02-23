// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyClangDiagnosticUnusedMacros
#pragma once


#ifdef _DEBUG
#define IMGUI_DEBUG_PARANOID
#else
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_DEBUG_TOOLS
#define IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

#define IMGUI_USE_WCHAR32
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H

//---

#ifdef IMGUI_USE_WCHAR32
#include <cstdint>
#define ImDrawIdx uint32_t
#endif

#if defined(_WIN32) && defined(_DEBUG)
struct IDirect3DTexture9;
#define ImTextureID IDirect3DTexture9*
#endif

#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
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
//--
#define ImPow   std::pow
#define ImLog   std::log
#define ImAbs   std::abs

template <std::floating_point T>
T ImSign(T x)
{
    if (x < 0)
        return -1;
    if (x > 0)
        return 1;
    return 0;
}

template <std::floating_point T>
T ImRsqrt(T x)
{
    return 1 / ImSqrt(x);
}
#endif