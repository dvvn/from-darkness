#include <fd/system.h>

// InstructionSet.cpp
// Compile by using: cl /EHsc /W4 InstructionSet.cpp
// processor: x86, x64
// Uses the __cpuid intrinsic to get information about
// CPU extended instruction set support.

#include <intrin.h>

#include <array>
#include <bitset>
#include <string>
#include <vector>

namespace fd::system
{
// ReSharper disable CppInconsistentNaming
struct InstructionSet_Internal
{
    InstructionSet_Internal()
    {
        // int cpuInfo[4] = {-1};
        std::array<int, 4> cpui;

        // Calling __cpuid with 0x0 as the function_id argument
        // gets the number of the highest valid function ID.
        __cpuid(cpui.data(), 0);
        nIds_ = cpui[0];

        for (int i = 0; i <= nIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            data_.push_back(cpui);
        }

        // Capture vendor string
        char vendor[0x20] = {};

        *reinterpret_cast<int*>(vendor)     = data_[0][1];
        *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
        *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
        vendor_.assign(vendor);
        if (vendor_.contains("Intel"))
            isIntel_ = true;
        else if (vendor_.contains("AMD"))
            isAMD_ = true;

        // load bitset with flags for function 0x00000001
        if (nIds_ >= 1)
        {
            f_1_ECX_ = data_[1][2];
            f_1_EDX_ = data_[1][3];
        }

        // load bitset with flags for function 0x00000007
        if (nIds_ >= 7)
        {
            f_7_EBX_ = data_[7][1];
            f_7_ECX_ = data_[7][2];
        }

        // Calling __cpuid with 0x80000000 as the function_id argument
        // gets the number of the highest valid extended ID.
        __cpuid(cpui.data(), 0x80000000);
        nExIds_ = cpui[0];

        char brand[0x40] = {};

        for (int i = 0x80000000; i <= nExIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            extdata_.push_back(cpui);
        }

        // load bitset with flags for function 0x80000001
        if (nExIds_ >= 0x80000001)
        {
            f_81_ECX_ = extdata_[1][2];
            f_81_EDX_ = extdata_[1][3];
        }

        // Interpret CPU brand string if reported
        if (nExIds_ >= 0x80000004)
        {
            memcpy(brand, extdata_[2].data(), sizeof(cpui));
            memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
            memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
            brand_.assign(brand);
        }
    };

    int nIds_;
    int nExIds_;

    std::string vendor_;
    std::string brand_;

    bool isIntel_ = false;
    bool isAMD_   = false;

    std::bitset<32> f_1_ECX_  = 0;
    std::bitset<32> f_1_EDX_  = 0;
    std::bitset<32> f_7_EBX_  = 0;
    std::bitset<32> f_7_ECX_  = 0;
    std::bitset<32> f_81_ECX_ = 0;
    std::bitset<32> f_81_EDX_ = 0;

    std::vector<std::array<int, 4>> data_;
    std::vector<std::array<int, 4>> extdata_;
};

class InstructionSet
{
    const InstructionSet_Internal CPU_Rep;

  public:
    // getters
    std::string Vendor() const
    {
        return CPU_Rep.vendor_;
    }

    std::string Brand() const
    {
        return CPU_Rep.brand_;
    }

    bool SSE3() const
    {
        return CPU_Rep.f_1_ECX_[0];
    }

    bool PCLMULQDQ() const
    {
        return CPU_Rep.f_1_ECX_[1];
    }

    bool MONITOR() const
    {
        return CPU_Rep.f_1_ECX_[3];
    }

    bool SSSE3() const
    {
        return CPU_Rep.f_1_ECX_[9];
    }

    bool FMA() const
    {
        return CPU_Rep.f_1_ECX_[12];
    }

    bool CMPXCHG16B() const
    {
        return CPU_Rep.f_1_ECX_[13];
    }

    bool SSE41() const
    {
        return CPU_Rep.f_1_ECX_[19];
    }

    bool SSE42() const
    {
        return CPU_Rep.f_1_ECX_[20];
    }

    bool MOVBE() const
    {
        return CPU_Rep.f_1_ECX_[22];
    }

    bool POPCNT() const
    {
        return CPU_Rep.f_1_ECX_[23];
    }

    bool AES() const
    {
        return CPU_Rep.f_1_ECX_[25];
    }

    bool XSAVE() const
    {
        return CPU_Rep.f_1_ECX_[26];
    }

    bool OSXSAVE() const
    {
        return CPU_Rep.f_1_ECX_[27];
    }

    bool AVX() const
    {
        return CPU_Rep.f_1_ECX_[28];
    }

    bool F16C() const
    {
        return CPU_Rep.f_1_ECX_[29];
    }

    bool RDRAND() const
    {
        return CPU_Rep.f_1_ECX_[30];
    }

    bool MSR() const
    {
        return CPU_Rep.f_1_EDX_[5];
    }

    bool CX8() const
    {
        return CPU_Rep.f_1_EDX_[8];
    }

    bool SEP() const
    {
        return CPU_Rep.f_1_EDX_[11];
    }

    bool CMOV() const
    {
        return CPU_Rep.f_1_EDX_[15];
    }

    bool CLFSH() const
    {
        return CPU_Rep.f_1_EDX_[19];
    }

    bool MMX() const
    {
        return CPU_Rep.f_1_EDX_[23];
    }

    bool FXSR() const
    {
        return CPU_Rep.f_1_EDX_[24];
    }

    bool SSE() const
    {
        return CPU_Rep.f_1_EDX_[25];
    }

    bool SSE2() const
    {
        return CPU_Rep.f_1_EDX_[26];
    }

    bool FSGSBASE() const
    {
        return CPU_Rep.f_7_EBX_[0];
    }

    bool BMI1() const
    {
        return CPU_Rep.f_7_EBX_[3];
    }

    bool HLE() const
    {
        return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4];
    }

    bool AVX2() const
    {
        return CPU_Rep.f_7_EBX_[5];
    }

    bool BMI2() const
    {
        return CPU_Rep.f_7_EBX_[8];
    }

    bool ERMS() const
    {
        return CPU_Rep.f_7_EBX_[9];
    }

    bool INVPCID() const
    {
        return CPU_Rep.f_7_EBX_[10];
    }

    bool RTM() const
    {
        return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11];
    }

    bool AVX512F() const
    {
        return CPU_Rep.f_7_EBX_[16];
    }

    bool RDSEED() const
    {
        return CPU_Rep.f_7_EBX_[18];
    }

    bool ADX() const
    {
        return CPU_Rep.f_7_EBX_[19];
    }

    bool AVX512PF() const
    {
        return CPU_Rep.f_7_EBX_[26];
    }

    bool AVX512ER() const
    {
        return CPU_Rep.f_7_EBX_[27];
    }

    bool AVX512CD() const
    {
        return CPU_Rep.f_7_EBX_[28];
    }

    bool SHA() const
    {
        return CPU_Rep.f_7_EBX_[29];
    }

    bool PREFETCHWT1() const
    {
        return CPU_Rep.f_7_ECX_[0];
    }

    bool LAHF() const
    {
        return CPU_Rep.f_81_ECX_[0];
    }

    bool LZCNT() const
    {
        return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5];
    }

    bool ABM() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5];
    }

    bool SSE4a() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6];
    }

    bool XOP() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11];
    }

    bool TBM() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21];
    }

    bool SYSCALL() const
    {
        return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11];
    }

    bool MMXEXT() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22];
    }

    bool RDTSCP() const
    {
        return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27];
    }

    bool _3DNOWEXT() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30];
    }

    bool _3DNOW() const
    {
        return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31];
    }
};

// ReSharper restore CppInconsistentNaming

#define INIT(_NAME_) _NAME_ = iset._NAME_();

cpu_info::cpu_info()
{
    const InstructionSet iset;

    INIT(Brand);
    INIT(Vendor);

    INIT(SSE3);
    INIT(PCLMULQDQ);
    INIT(MONITOR);
    INIT(SSSE3);
    INIT(FMA);
    INIT(CMPXCHG16B);
    INIT(SSE41);
    INIT(SSE42);
    INIT(MOVBE);
    INIT(POPCNT);
    INIT(AES);
    INIT(XSAVE);
    INIT(OSXSAVE);
    INIT(AVX);
    INIT(F16C);
    INIT(RDRAND);

    INIT(MSR);
    INIT(CX8);
    INIT(SEP);
    INIT(CMOV);
    INIT(CLFSH);
    INIT(MMX);
    INIT(FXSR);
    INIT(SSE);
    INIT(SSE2);

    INIT(FSGSBASE);
    INIT(BMI1);
    INIT(HLE);
    INIT(AVX2);
    INIT(BMI2);
    INIT(ERMS);
    INIT(INVPCID);
    INIT(RTM);
    INIT(AVX512F);
    INIT(RDSEED);
    INIT(ADX);
    INIT(AVX512PF);
    INIT(AVX512ER);
    INIT(AVX512CD);
    INIT(SHA);

    INIT(PREFETCHWT1);

    INIT(LAHF);
    INIT(LZCNT);
    INIT(ABM);
    INIT(SSE4a);
    INIT(XOP);
    INIT(TBM);

    INIT(SYSCALL);
    INIT(MMXEXT);
    INIT(RDTSCP);
    INIT(_3DNOWEXT);
    INIT(_3DNOW);
}

const cpu_info CPU;

} // namespace fd::system