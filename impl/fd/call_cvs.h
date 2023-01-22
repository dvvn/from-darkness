#pragma once
#include <cstdint>

namespace fd
{
#undef cdecl
enum class call_cvs : uint8_t
{
    // ReSharper disable CppInconsistentNaming
    thiscall_,
    cdecl_,
    fastcall_,
    stdcall_,
    vectorcall_,
    // ReSharper restore CppInconsistentNaming
};
}