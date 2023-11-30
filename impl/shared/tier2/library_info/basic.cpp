#include "tier0/iterator/unwrap.h"
#include "tier1/diagnostics/fatal.h"
#include "tier1/functional/cast.h"
#include "tier2/library_info/basic.h"

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

// template <typename It>
// static bool equal(UNICODE_STRING const& ustr, size_t const offset, It first, It last)
//{
//     return std::equal(begin(ustr) + offset, end(ustr), first, last);
// }
//
template <typename It>
static bool equal(UNICODE_STRING const& ustr, It first, It last) requires(sizeof(std::iter_value_t<It>) == sizeof(wchar_t))
{
    return std::equal(begin(ustr), end(ustr), first, last);
}

template <typename It>
static bool equal(UNICODE_STRING const& ustr, It first1, It last1, It first2, It last2) requires(sizeof(std::iter_value_t<It>) == sizeof(wchar_t))
{
    auto const length1 = std::distance(first1, last1);
    auto const length2 = std::distance(first2, last2);
    if (size(ustr) != length1 + length2)
        return false;
    auto const check1 = [=] {
        return std::equal(begin(ustr), end(ustr) - length2, first1, last1);
    };
    auto const check2 = [=] {
        return std::equal(begin(ustr) + length1, end(ustr), first2, last2);
    };
    return length1 <= length2 ? check1() && check2() : check2() && check1();
}

template <typename It>
static bool equal(IMAGE_SECTION_HEADER const& header, It first, It last) requires(sizeof(std::iter_value_t<It>) == sizeof(BYTE))
{
    size_t const length = std::distance(first, last);
    assert(length < std::size(header.Name));

    return header.Name[length] == '\0' && std::equal(header.Name, header.Name + length, first, last);
}

namespace FD_TIER(2)
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

wstring_view basic_library_info::extension_tag::get() const
{
#ifdef _DEBUG
    return to_string(value_);
#else
    return ext_;
#endif
}

basic_library_info::basic_library_info(wstring_view name)
{
    entry_full_ = find_library(ubegin(name), uend(name));
}

basic_library_info::basic_library_info(wstring_view name, extension_tag const ext)
{
    auto const ext_str = ext.get();
    entry_full_        = find_library(ubegin(name), uend(name), ubegin(ext_str), uend(ext_str));
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

wstring_view basic_library_info::name() const
{
    auto const& buff = entry_full_->BaseDllName;
    return {begin(buff), end(buff)};
}

wstring_view basic_library_info::path() const
{
    auto const& buff = entry_full_->FullDllName;
    return {begin(buff), end(buff)};
}

auto basic_library_info::sections() const -> sections_range
{
    return {nt_header()};
}

auto basic_library_info::data_dirs() const -> data_dir_range
{
    return {nt_header()};
}

basic_library_info::data_dir_range::data_dir_range(IMAGE_NT_HEADERS* nt)
    : span(nt->OptionalHeader.DataDirectory)
{
}

basic_library_info::sections_range::sections_range(IMAGE_NT_HEADERS* nt)
    : span(IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections)
{
}

basic_library_info::section_view::section_view(IMAGE_SECTION_HEADER const* section, void* image_base)
    : span(safe_cast<uint8_t>(image_base) + section->VirtualAddress, section->SizeOfRawData)
{
}

IMAGE_SECTION_HEADER* find(basic_library_info::sections_range sections, string_view name)
{
    auto const sections_end = uend(sections);
    auto const found        = std::find_if(ubegin(sections), sections_end, [first = ubegin(name), last = uend(name)](IMAGE_SECTION_HEADER const& header) {
        return equal(header, first, last);
    });
    return found == sections_end ? nullptr : found;
}

uint8_t* begin(basic_library_info const& info)
{
    return safe_cast_from(info.image_base());
}

uint8_t* end(basic_library_info const& info)
{
    return begin(info) + info.length();
}
} // namespace FD_TIER(2)
