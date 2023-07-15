﻿#include "basic.h"
#include "string/view.h"

#include <windows.h>
#include <winternl.h>

#include <cassert>

static wchar_t const *data(UNICODE_STRING const &ustr)
{
    return ustr.Buffer;
}

static size_t size(UNICODE_STRING const &ustr)
{
    return ustr.Length / sizeof(wchar_t);
}

static wchar_t const *begin(UNICODE_STRING const &ustr)
{
    return ustr.Buffer;
}

static wchar_t const *end(UNICODE_STRING const &ustr)
{
    return ustr.Buffer + size(ustr);
}

// ReSharper disable CppInconsistentNaming
struct _LDR_DATA_TABLE_ENTRY_FULL
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
};

// ReSharper restore CppInconsistentNaming

static bool operator!(UNICODE_STRING const &ustr)
{
    return ustr.Buffer == nullptr;
}

static bool equal(wchar_t const *str, size_t const length, UNICODE_STRING const &ustr)
{
    return length == size(ustr) && memcmp(str, data(ustr), length) == 0;
}

namespace fd
{
basic_library_info::basic_library_info(char_type const *name, size_t const length)
{
#ifdef _WIN64
    auto mem = NtCurrentTeb();
    auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto mem = reinterpret_cast<PEB *>(__readfsdword(0x30));
    auto ldr = mem->Ldr;
#endif
    auto const root_list = &ldr->InMemoryOrderModuleList;

    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        auto const entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY_FULL, InMemoryOrderLinks);
        if (!entry->BaseDllName)
            continue;
        if (!equal(name, length, entry->BaseDllName))
            continue;

        entry_full_ = entry;
        return;
    }

#ifdef _DEBUG
    entry_ = nullptr;
#endif
}

void *basic_library_info::base() const
{
    return entry_full_->DllBase;
}

#define DOS_NT                                                                                   \
    auto dos = entry_full_->DosHeader;                                                           \
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE);                                                 \
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS *>(entry_full_->DllBaseAddress + dos->e_lfanew); \
    assert(nt->Signature == IMAGE_NT_SIGNATURE);

void *basic_library_info::image_base() const
{
    DOS_NT;
    return reinterpret_cast<void *>(nt->OptionalHeader.ImageBase);
}

size_t basic_library_info::length() const
{
    return entry_full_->SizeOfImage;
}

auto basic_library_info::name() const -> string_type
{
    auto const buff = entry_full_->BaseDllName;
    return {begin(buff), end(buff)};
}

auto basic_library_info::path() const -> string_type
{
    auto const buff = entry_full_->FullDllName;
    return {begin(buff), end(buff)};
}

IMAGE_DATA_DIRECTORY *basic_library_info::directory(uint8_t const index) const
{
    DOS_NT;
    return nt->OptionalHeader.DataDirectory + index;
}

IMAGE_SECTION_HEADER *basic_library_info::section(char const *name, uint8_t const length) const
{
    DOS_NT;

    auto begin     = IMAGE_FIRST_SECTION(nt);
    auto const end = begin + nt->FileHeader.NumberOfSections;

    for (; begin != end; ++begin)
    {
        if (begin->Name[length] == '\0' && // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            memcmp(begin->Name, name, length) == 0)
            return begin;
    }

    return nullptr;
}
} // namespace fd