#pragma once

#include <fd/string.h>

namespace fd::system
{
struct cpu_info
{
    cpu_info();

    string Brand, Vendor;

    // ReSharper disable CppInconsistentNaming
    bool SSE3;
    bool PCLMULQDQ;
    bool MONITOR;
    bool SSSE3;
    bool FMA;
    bool CMPXCHG16B;
    bool SSE41;
    bool SSE42;
    bool MOVBE;
    bool POPCNT;
    bool AES;
    bool XSAVE;
    bool OSXSAVE;
    bool AVX;
    bool F16C;
    bool RDRAND;

    bool MSR;
    bool CX8;
    bool SEP;
    bool CMOV;
    bool CLFSH;
    bool MMX;
    bool FXSR;
    bool SSE;
    bool SSE2;

    bool FSGSBASE;
    bool BMI1;
    bool HLE;
    bool AVX2;
    bool BMI2;
    bool ERMS;
    bool INVPCID;
    bool RTM;
    bool AVX512F;
    bool RDSEED;
    bool ADX;
    bool AVX512PF;
    bool AVX512ER;
    bool AVX512CD;
    bool SHA;

    bool PREFETCHWT1;

    bool LAHF;
    bool LZCNT;
    bool ABM;
    bool SSE4a;
    bool XOP;
    bool TBM;

    bool SYSCALL;
    bool MMXEXT;
    bool RDTSCP;
    bool _3DNOWEXT;
    bool _3DNOW;
    // ReSharper restore CppInconsistentNaming
};

extern const cpu_info CPU;
}