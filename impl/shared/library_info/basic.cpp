﻿#include "library_info/basic.h"
#include "diagnostics/fatal.h"
#include "functional/cast.h"
#include "iterator/unwrap.h"

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

static bool equal(UNICODE_STRING const& ustr, size_t const offset, wchar_t const* str, size_t const length)
{
    return memcmp(data(ustr) + offset, str, length * sizeof(wchar_t)) == 0;
}

static bool equal(UNICODE_STRING const& ustr, wchar_t const* str, size_t const length)
{
    return size(ustr) == length && equal(ustr, 0, str, length);
}

static bool equal(UNICODE_STRING const& ustr, wchar_t const* part1, size_t const part1_length, wchar_t const* part2, size_t const part2_length)
{
    if (size(ustr) != part1_length + part2_length)
        return false;
    auto const check1 = [=] {
        return equal(ustr, 0, part1, part1_length);
    };
    auto const check2 = [=] {
        return equal(ustr, part1_length, part2, part2_length);
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
template <typename... T>
static LDR_DATA_TABLE_ENTRY_FULL* find_library(T... args)
#ifdef _DEBUG
    requires requires(UNICODE_STRING str) { equal(str, args...); }
#endif
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

basic_library_info::basic_library_info(char const* name, size_t const length, wchar_t* buffer)
{
    std::transform(name, name + length, buffer, tolower);
    entry_full_ = find_library(buffer, length);
}

basic_library_info::basic_library_info(LPCTSTR const name, size_t const length)
{
    entry_full_ = (find_library(name, length));
}

// basic_library_info::basic_library_info(char const* name, size_t const name_length, wchar_t* name_buffer, wchar_t const* extension, size_t const
// extension_length)
//{
// #ifdef _DEBUG
//     validate_library_name(extension, extension_length);
// #endif
//     std::transform(name, name + name_length, name_buffer, tolower);
//     entry_full_ = find_library(name_buffer, name_length, extension, extension_length);
// }

basic_library_info::basic_library_info(wchar_t const* name, size_t const name_length, wchar_t const* extension, size_t const extension_length)
{
    entry_full_ = find_library(name, name_length, extension, extension_length);
}

basic_library_info::basic_library_info(wchar_t const* name, size_t const length, extension_tag)
    : basic_library_info(name, length, L".dll", 4)
{
}

IMAGE_DOS_HEADER* basic_library_info::dos_header() const
{
    auto const dos = entry_full_->DosHeader;
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE);
    return dos;
}

IMAGE_NT_HEADERS* basic_library_info::nt_header() const
{
    return nt_header(dos_header());
}

IMAGE_NT_HEADERS* basic_library_info::nt_header(IMAGE_DOS_HEADER const* dos) const
{
    auto const nt = unsafe_cast<IMAGE_NT_HEADERS*>(entry_full_->DllBaseAddress + dos->e_lfanew);
    assert(nt->Signature == IMAGE_NT_SIGNATURE);
    return nt;
}

library_base_address basic_library_info::base() const
{
    return {entry_full_->DllBase};
}

void* basic_library_info::image_base(IMAGE_NT_HEADERS* nt)
{
    return unsafe_cast<void*>(nt->OptionalHeader.ImageBase);
}

void* basic_library_info::image_base() const
{
    return image_base(nt_header());
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

uint8_t* begin(basic_library_info const& info)
{
    return safe_cast<uint8_t>(info.image_base());
}

uint8_t* end(basic_library_info const& info)
{
    return begin(info) + info.length();
}

library_section_view::library_section_view(IMAGE_SECTION_HEADER const* section, void* image_base)
    : span(safe_cast<uint8_t>(image_base) + section->VirtualAddress, section->SizeOfRawData)
{
}

library_sections_range::library_sections_range(IMAGE_NT_HEADERS* nt)
    : span(IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections)
{
}

library_data_dir_range::library_data_dir_range(IMAGE_NT_HEADERS* nt)
    : span(nt->OptionalHeader.DataDirectory)
{
}

IMAGE_SECTION_HEADER* find(library_sections_range sections, char const* name, size_t const name_length)
{
    return std::find_if(ubegin(sections), uend(sections), [=](IMAGE_SECTION_HEADER const& header) {
        return equal(header, name, name_length);
    });
}

namespace detail
{
library_section_view make_section_view(void const* section, void* image_base)
{
    return {safe_cast<IMAGE_SECTION_HEADER>(section), image_base};
}

library_section_view make_section_view(IMAGE_SECTION_HEADER const* section, void* image_base)
{
    return {section, image_base};
}

library_sections_range make_sections_range(void* nt)
{
    auto nt1 = safe_cast<IMAGE_NT_HEADERS>(nt);
    assert(nt1->Signature == IMAGE_NT_SIGNATURE);
    return {nt1};
}

library_sections_range make_sections_range(IMAGE_NT_HEADERS* nt)
{
    return {nt};
}
} // namespace detail
} // namespace fd
