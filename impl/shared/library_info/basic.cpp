#include "library_info/basic.h"
#include "functional/cast.h"

#include <algorithm>
#include <cassert>

static wchar_t const* data(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

static size_t size(UNICODE_STRING const& ustr)
{
    return ustr.Length / sizeof(wchar_t);
}

static wchar_t const* begin(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

static wchar_t const* end(UNICODE_STRING const& ustr)
{
    return ustr.Buffer + size(ustr);
}

static bool operator!(UNICODE_STRING const& ustr)
{
    return ustr.Buffer == nullptr;
}

static bool equal(UNICODE_STRING const& ustr, wchar_t const* str, size_t const length)
{
    return size(ustr) == length && memcmp(str, data(ustr), length) == 0;
}

namespace fd
{
basic_library_info::basic_library_info(char_type const* name, size_t const length)
{
    assert(!std::any_of(name, name + length, isupper));

#ifdef _WIN64
    auto const mem = reinterpret_cast<TEB*>(__readgsqword(FIELD_OFFSET(NT_TIB64, Self)));
    auto const ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto const mem = reinterpret_cast<PEB*>(__readfsdword(FIELD_OFFSET(NT_TIB32, Self)));
    auto const ldr = mem->Ldr;
#endif

    auto const root_list = &ldr->InMemoryOrderModuleList;
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        auto const entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY_FULL, InMemoryOrderLinks);
        if (!entry->BaseDllName)
            continue;
        if (!equal(entry->BaseDllName, name, length))
            continue;

        entry_full_ = entry;
        return;
    }

#ifdef _DEBUG
    entry_ = nullptr;
#endif
}

void* basic_library_info::base() const
{
    return entry_full_->DllBase;
}

#define SET_DOS                              \
    auto const dos = entry_full_->DosHeader; \
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE);

#define SET_NT                                                                                   \
    SET_DOS;                                                                                     \
    auto const nt = unsafe_cast<IMAGE_NT_HEADERS*>(entry_full_->DllBaseAddress + dos->e_lfanew); \
    assert(nt->Signature == IMAGE_NT_SIGNATURE);

void* basic_library_info::image_base() const
{
    SET_NT;
    return unsafe_cast<void*>(nt->OptionalHeader.ImageBase);
}

size_t basic_library_info::length() const
{
    return entry_full_->SizeOfImage;
}

auto basic_library_info::name() const -> string_type
{
    auto const& buff = entry_full_->BaseDllName;
    return {begin(buff), end(buff)};
}

auto basic_library_info::path() const -> string_type
{
    auto const& buff = entry_full_->FullDllName;
    return {begin(buff), end(buff)};
}

IMAGE_DATA_DIRECTORY* basic_library_info::directory(uint8_t const index) const
{
    SET_NT;
    return nt->OptionalHeader.DataDirectory + index;
}

IMAGE_SECTION_HEADER* basic_library_info::section(char const* name, uint8_t const length) const
{
    assert(length < sizeof(IMAGE_SECTION_HEADER::Name));

    SET_NT;

    auto begin     = IMAGE_FIRST_SECTION(nt);
    auto const end = begin + nt->FileHeader.NumberOfSections;

    for (; begin != end; ++begin)
    {
        if (begin->Name[length] != '\0') // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            continue;
        if (memcmp(begin->Name, name, length) != 0)
            continue;
        return begin;
    }

    return nullptr;
}

uint8_t* begin(basic_library_info const& info)
{
    return static_cast<uint8_t*>(info.image_base());
}

uint8_t* end(basic_library_info const& info)
{
    return begin(info) + info.length();
}

span<uint8_t> make_library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base)
{
    auto const section_start = safe_cast<uint8_t>(image_base) + section->VirtualAddress;
    auto const section_end   = section_start + section->SizeOfRawData;

    return {section_start, section_end};
}

library_section_view::library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base)
    : span(make_library_section_view(section, image_base))
{
}
} // namespace fd
