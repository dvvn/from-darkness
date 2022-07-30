module;

#include <fd/assert.h>

#include <fd/rt_modules/winapi.h>

#include <span>

module fd.rt_modules:library_info;
import fd.logger;
import fd.path;

void dos_nt::_Construct(const LDR_DATA_TABLE_ENTRY* const ldr_entry)
{
    FD_ASSERT(ldr_entry != nullptr);
    dos = (IMAGE_DOS_HEADER*)ldr_entry->DllBase;
    // check for invalid DOS / DOS signature.
    FD_ASSERT(dos && dos->e_magic == IMAGE_DOS_SIGNATURE /* 'MZ' */);
    nt = map<IMAGE_NT_HEADERS>(dos->e_lfanew);
    // check for invalid NT / NT signature.
    FD_ASSERT(nt && nt->Signature == IMAGE_NT_SIGNATURE /* 'PE\0\0' */);
}

dos_nt::dos_nt(const LDR_DATA_TABLE_ENTRY* const ldr_entry)
{
    _Construct(ldr_entry);
}

dos_nt::dos_nt(const library_info info)
{
    _Construct(std::addressof(*info));
}

std::span<uint8_t> dos_nt::read() const
{
    return { (uint8_t*)dos, nt->OptionalHeader.SizeOfImage };
}

std::span<IMAGE_SECTION_HEADER> dos_nt::sections() const
{
    return { IMAGE_FIRST_SECTION(nt), nt->FileHeader.NumberOfSections };
}

//---------

library_info::library_info(pointer const entry)
    : entry_(entry)
{
    FD_ASSERT(entry != nullptr);
}

auto library_info::operator->() const -> pointer
{
    return entry_;
}

auto library_info::operator*() const -> reference
{
    return *entry_;
}

fd::wstring_view library_info::path() const
{
    return { entry_->FullDllName.Buffer, entry_->FullDllName.Length / sizeof(WCHAR) };
}

fd::wstring_view library_info::name() const
{
    const auto full_path = this->path();
#if 1
    return fd::path<wchar_t>(full_path).filename();
#else
    const auto name_start = full_path.rfind('\\');
    FD_ASSERT(name_start != full_path.npos, "Unable to get the module name");
    return full_path.substr(name_start + 1);
#endif
}

void library_info::log(const fd::string_view object_type, const fd::string_view object, const void* addr) const
{
    fd::invoke(fd::logger,
               L"{} -> {} '{}' {}! ({:#X})",
               fd::bind_front(&library_info::name, this),
               object_type,
               object,
               addr ? L"found" : L"not found",
               reinterpret_cast<uintptr_t>(addr));
}
