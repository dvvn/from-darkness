#pragma once

#include <cstdint>

#define C_NONE    0x00
#define C_MODRM   0x01
#define C_IMM8    0x02
#define C_IMM16   0x04
#define C_IMM_P66 0x10
#define C_REL8    0x20
#define C_REL32   0x40
#define C_GROUP   0x80
#define C_ERROR   0xff

#define PRE_ANY  0x00
#define PRE_NONE 0x01
#define PRE_F2   0x02
#define PRE_F3   0x04
#define PRE_66   0x08
#define PRE_67   0x10
#define PRE_LOCK 0x20
#define PRE_SEG  0x40
#define PRE_ALL  0xff

#if defined(_M_IX86) || defined(__i386__)
#define DHOOKS_X86
#elif defined(_M_X64) || defined(__x86_64__)
#define DHOOKS_X64
#else
#error Only x86 and x64 operation systems supported.
#endif

#ifndef  __cplusplus
#error C++ compiler required.
#endif

#ifndef _INC_DDEMLH
#define _INC_DDEMLH
#endif
