#include "library_info.h"

#include <algorithm>
#include <cassert>

PWSTR data(UNICODE_STRING&& ustr)  = delete;
USHORT size(UNICODE_STRING&& ustr) = delete;
PWSTR begin(UNICODE_STRING&& ustr) = delete;
PWSTR end(UNICODE_STRING&& ustr)   = delete;

static PWSTR data(UNICODE_STRING const& ustr) noexcept
{
    return ustr.Buffer;
}

static USHORT size(UNICODE_STRING const& ustr) noexcept
{
    return ustr.Length / sizeof(WCHAR);
}

static PWSTR begin(UNICODE_STRING const& ustr) noexcept
{
    return ustr.Buffer;
}

static PWSTR end(UNICODE_STRING const& ustr) noexcept
{
    return ustr.Buffer + size(ustr);
}

static bool operator!(UNICODE_STRING const& ustr) noexcept
{
    return ustr.Buffer == nullptr;
}

#ifdef _MSC_VER
static int wmemcmp2(wchar_t const* buff1, wchar_t const* buff2, size_t const size)
{
    return memcmp(buff1, buff2, size * sizeof(wchar_t));
}
#else
inline constexpr auto wmemcmp2 = wmemcmp;
#endif

static bool equal(UNICODE_STRING const& ustr, std::wstring_view const name)
{
    if (!ustr)
        return false;
    if (size(ustr) != size(name))
        return false;

    if (wmemcmp2(data(ustr), data(name), size(name)) != 0)
        return false;

    return true;
}

static bool equal(UNICODE_STRING const& ustr, std::wstring_view const name, std::wstring_view const extension)
{
    if (!ustr)
        return false;
    if (size(ustr) != size(name) + size(extension))
        return false;

    if (wmemcmp2(data(ustr), data(name), size(name)) != 0)
        return false;
    if (wmemcmp2(data(ustr) + size(name), data(extension), size(extension)) != 0)
        return false;

    return true;
}

namespace fd
{
static LIST_ENTRY* module_list() noexcept
{
#ifdef _WIN64
    auto const mem = reinterpret_cast<TEB*>(__readgsqword(FIELD_OFFSET(NT_TIB64, Self)));
    auto const ldr = mem->ProcessEnvironmentBlock->Ldr;
#else
    auto const mem = reinterpret_cast<PEB*>(__readfsdword(FIELD_OFFSET(NT_TIB32, Self)));
    auto const ldr = mem->Ldr;
#endif

    return &ldr->InMemoryOrderModuleList;
}

template <class T>
static T* ldr_table(LIST_ENTRY* entry) noexcept
{
    return CONTAINING_RECORD(entry, T, InMemoryOrderLinks);
}

template <typename Cmp>
static PLDR_DATA_TABLE_ENTRY_FULL find_module_entry(Cmp cmp)
{
    auto const root_list = module_list();
    for (auto list_entry = root_list->Flink; list_entry != root_list; list_entry = list_entry->Flink)
    {
        auto const entry = ldr_table<LDR_DATA_TABLE_ENTRY_FULL>(list_entry);
        if (cmp(entry->BaseDllName))
            return entry;
    }
    return nullptr;
}

library_info::library_info(PLDR_DATA_TABLE_ENTRY_FULL entry)
    : entry_full_(entry)
{
}

library_info::library_info(PLDR_DATA_TABLE_ENTRY entry)
    : entry_(entry)
{
}

library_info::library_info(wchar_t const* name, size_t const length)
    : library_info(std::wstring_view{name, length})
{
}

library_info::library_info(std::wstring_view const name)
    : library_info(find_module_entry([name](UNICODE_STRING const& module_name) {
        return equal(module_name, name);
    }))
{
}

library_info::library_info(std::wstring_view const name, std::wstring_view const extension)
    : library_info(find_module_entry([name, extension](UNICODE_STRING const& module_name) {
        return equal(module_name, name, extension);
    }))
{
}

library_info::operator bool() const
{
    return entry_ != nullptr;
}

IMAGE_DOS_HEADER* library_info::dos_header() const
{
    auto const dos = entry_full_->DosHeader;
    assert(dos->e_magic == IMAGE_DOS_SIGNATURE);
    return dos;
}

IMAGE_NT_HEADERS* library_info::nt_header() const
{
    auto const dos = dos_header();
    auto const nt  = unsafe_cast<IMAGE_NT_HEADERS*>(entry_full_->DllBaseAddress + dos->e_lfanew);
    assert(nt->Signature == IMAGE_NT_SIGNATURE);
    return nt;
}

void* library_info::image_base() const
{
    auto const nt = nt_header();
    return unsafe_cast_from(nt->OptionalHeader.ImageBase);
}

uint8_t* library_info::data() const
{
    return safe_cast_from(entry_full_->DllBase);
}

ULONG library_info::size() const
{
    return entry_full_->SizeOfImage;
}

std::wstring_view library_info::name() const
{
    using std::data;
    using std::size;
    auto const& buff = entry_full_->BaseDllName;
    return {data(buff), size(buff)};
}

std::wstring_view library_info::path() const
{
    using std::data;
    using std::size;
    auto const& buff = entry_full_->FullDllName;
    return {data(buff), size(buff)};
}

library_info literals::operator""_dll(wchar_t const* name, size_t const length)
{
    return find_module_entry([name1 = std::wstring_view(name, length), extension = std::wstring_view(L".dll")](UNICODE_STRING const& module_name) {
        return equal(module_name, name1, extension);
    });
}

#if 0
 void* library_info::vtable(std::string_view name) const
{
    union
    {
        uintptr_t rtti_descriptor_address;
        void* rtti_descriptor;
        char const* rtti_descriptor_view;
    };

    // rtti_descriptor = find_rtti_descriptor(name, image_base, nt_header);
    if (auto const space = name.find(' '); space == name.npos)
    {
        rtti_descriptor = find(make_pattern(".?A", 1, name, 0, "@@"));
    }
    else
    {
        auto const object_name = name.substr(0, space);
        char object_tag;

        if (object_name == "struct")
            object_tag = 'U';
        else if (object_name == "class")
            object_tag = 'V';
        else
            unreachable();

        auto const class_name = name.substr(space + 1);

        rtti_descriptor = find(make_pattern(".?A", 0, object_tag, 0, class_name, 0, "@@"));
    }

    //---------

    // we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the std::string
    xref const type_descriptor{rtti_descriptor_address - sizeof(uintptr_t) * 2};

    // dos + section->VirtualAddress, section->SizeOfRawData

    auto const img_base = safe_cast<uint8_t>(image_base());

    auto const rdata       = find(section::rdata);
    auto const rdata_first = img_base + rdata->VirtualAddress;
    auto const rdata_last  = rdata_first + rdata->SizeOfRawData;

    auto const addr1 = std::find_if(rdata_first, rdata_last, [&type_descriptor](uint8_t const& found) -> bool {
        return std::equal(type_descriptor.begin(), type_descriptor.end(), &found) && *unsafe_cast<uint32_t*>(&found - 0x8) == 0;
    });
    if (addr1 == rdata_last)
        return nullptr;
    xref const unnamed_xref2{unsafe_cast<uintptr_t>(addr1) - 0xC};
    auto const addr2 = std::search(rdata_first, rdata_last, unnamed_xref2.begin(), unnamed_xref2.end());
    if (addr2 == rdata_last)
        return nullptr;

    auto const text       = find(section::text);
    auto const text_first = img_base + text->VirtualAddress;
    auto const text_last  = text_first + text->SizeOfRawData;
    xref const unnamed_xref3{unsafe_cast<uintptr_t>(addr2) + 4};
    auto const addr3 = std::search(text_first, text_last, unnamed_xref3.begin(), unnamed_xref3.end());
    if (addr3 == text_last)
        return nullptr;

    return (addr3);
}
#endif

} // namespace fd