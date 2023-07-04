#include "basic.h"
#include "string/view.h"

#include <windows.h>
#include <winternl.h>

#include <cassert>

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

struct UNICODE_STRING_view : std::wstring_view
{
    UNICODE_STRING_view(UNICODE_STRING const &ustr)
        : std::wstring_view(ustr.Buffer, ustr.Length / sizeof ustr.Buffer[0])
    {
    }
};

// ReSharper restore CppInconsistentNaming

static bool operator==(std::wstring_view str, UNICODE_STRING_view ustr)
{
    return std::operator==(str, ustr);
}

static bool operator!(UNICODE_STRING const &ustr)
{
    return ustr.Buffer == nullptr;
}

namespace fd
{

basic_library_info::basic_library_info(string_type name)
{
#ifdef _WIN64
    auto mem = NtCurrentTeb();
    auto ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto mem = reinterpret_cast<PEB *>(__readfsdword(0x30));
    auto ldr = mem->Ldr;
#endif
    auto root_list = &ldr->InMemoryOrderModuleList;

    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        auto entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY_FULL, InMemoryOrderLinks);
        if (!entry->BaseDllName)
            continue;
        if (name != entry->BaseDllName)
            continue;

        entry_full_ = entry;
        return;
    }

#ifdef _DEBUG
    entry_ = nullptr;
#endif
}

basic_library_info::basic_library_info(const char_type *name, size_t length)
    : basic_library_info(string_type{name, length})
{
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
    return UNICODE_STRING_view(entry_full_->BaseDllName);
}

auto basic_library_info::path() const -> string_type
{
    return UNICODE_STRING_view(entry_->FullDllName);
}

IMAGE_DATA_DIRECTORY *basic_library_info::directory(uint8_t index) const
{
    DOS_NT;
    return nt->OptionalHeader.DataDirectory + index;
}

IMAGE_SECTION_HEADER *basic_library_info::section(string_view name) const
{
    DOS_NT;

    auto begin = IMAGE_FIRST_SECTION(nt);
    auto end   = begin + nt->FileHeader.NumberOfSections;

    auto name_length = name.length();

    for (; begin != end; ++begin)
    {
        if (begin->Name[name_length] == '\0' && memcmp(begin->Name, name.data(), name_length) == 0)
            return begin;
    }

    return nullptr;
}
} // namespace fd