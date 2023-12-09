#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <windows.h>
#include <winternl.h>

// ReSharper disable CppInconsistentNaming

// see https://www.vergiliusproject.com/kernels/x64/Windows%2011/22H2%20(2022%20Update)/_LDR_DATA_TABLE_ENTRY
typedef struct _LDR_DATA_TABLE_ENTRY_FULL
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;

    union
    {
        PVOID DllBase;
        PIMAGE_DOS_HEADER DosHeader;
        ULONG_PTR DllBaseAddress;
    };

    PVOID EntryPoint;

    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_FULL, *PLDR_DATA_TABLE_ENTRY_FULL;

// ReSharper restore CppInconsistentNaming

PWSTR data(UNICODE_STRING&& ustr)  = delete;
USHORT size(UNICODE_STRING&& ustr) = delete;
PWSTR begin(UNICODE_STRING&& ustr) = delete;
PWSTR end(UNICODE_STRING&& ustr)   = delete;

inline PWSTR data(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

inline USHORT size(UNICODE_STRING const& ustr)
{
    return ustr.Length / sizeof(WCHAR);
}

inline PWSTR begin(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

inline PWSTR end(UNICODE_STRING const& ustr)
{
    return ustr.Buffer + size(ustr);
}

inline bool operator!(UNICODE_STRING const& ustr)
{
    return ustr.Buffer == nullptr;
}
