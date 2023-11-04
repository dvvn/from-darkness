#include "library_info/basic.h"
#include "functional/cast.h"
#include "string/char.h"

#include <algorithm>
#include <cassert>

static wchar_t* data(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

static size_t size(UNICODE_STRING const& ustr)
{
    return ustr.Length / sizeof(wchar_t);
}

static wchar_t* begin(UNICODE_STRING const& ustr)
{
    return ustr.Buffer;
}

static wchar_t* end(UNICODE_STRING const& ustr)
{
    return ustr.Buffer + size(ustr);
}

static bool operator!(UNICODE_STRING const& ustr)
{
    return ustr.Buffer == nullptr;
}

static bool equal(UNICODE_STRING const& ustr, wchar_t const* str, size_t const length)
{
    return std::equal(begin(ustr), end(ustr), str, str + length);
}

static bool equal(UNICODE_STRING const& ustr, wchar_t const* part1, size_t const part1_length, wchar_t const* part2, size_t const part2_length)
{
    if (size(ustr) != part1_length + part2_length)
        return false;
    auto const check1 = [=] {
        return std::equal(begin(ustr), end(ustr), part1, part1 + part1_length);
    };
    auto const check2 = [=] {
        return std::equal(begin(ustr) + part1_length, end(ustr), part2, part2 + part2_length);
    };
    return part1_length <= part2_length ? check1() && check2() : check2() && check1();
}

static bool equal(IMAGE_SECTION_HEADER const& header, char const* name, size_t const length)
{
    assert(length < std::size(header.Name));

    if (header.Name[length] != '\0')
        return false;
    if (memcmp(header.Name, name, length) != 0)
        return false;
    return true;
}

namespace fd
{
template <typename Fn>
static void validate_library_name(wchar_t const* name, size_t const length, Fn isupper_check)
{
    assert(!std::any_of(name, name + length, isupper_check));
}

static void validate_library_name(wchar_t const* name, size_t const length)
{
    if constexpr (std::invocable<decltype(isupper), wchar_t>)
        validate_library_name(name, length, isupper);
    else
        validate_library_name(name, length, iswupper);
}

template <typename... T>
static LDR_DATA_TABLE_ENTRY_FULL* find_library(T... args)
{
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
        if (!equal(entry->BaseDllName, args...))
            continue;

        return entry;
    }

    return nullptr;
}

basic_library_info::basic_library_info(LPCTSTR const name, size_t const length)
    : entry_full_(find_library(name, length))
{
#ifdef _DEBUG
    if (!entry_)
        validate_library_name(name, length);
#endif
}

basic_library_info::basic_library_info(wchar_t const* name, size_t const name_length, wchar_t const* extension, size_t const extension_length)
    : entry_full_(find_library(name, name_length, extension, extension_length))
{
#ifdef _DEBUG
    if (!entry_)
        validate_library_name(name, name_length + extension_length);
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

auto basic_library_info::name() const -> wstring_view
{
    auto const& buff = entry_full_->BaseDllName;
    return {begin(buff), end(buff)};
}

auto basic_library_info::path() const -> wstring_view
{
    auto const& buff = entry_full_->FullDllName;
    return {begin(buff), end(buff)};
}

IMAGE_DATA_DIRECTORY* basic_library_info::directory(size_t const index) const
{
    SET_NT;
    return nt->OptionalHeader.DataDirectory + index;
}

IMAGE_SECTION_HEADER* basic_library_info::section(char const* name, size_t const length) const
{
    SET_NT;

    auto begin     = IMAGE_FIRST_SECTION(nt);
    auto const end = begin + nt->FileHeader.NumberOfSections;

    for (; begin != end; ++begin)
    {
        if (equal(*begin, name, length))
            return begin;
    }

    return nullptr;
}

uint8_t* begin(basic_library_info const& info)
{
    return safe_cast<uint8_t>(info.image_base());
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
