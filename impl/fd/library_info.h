#pragma once

#include <fd/string.h>
#include <fd/type_name.h>

#include <windows.h>
#include <winternl.h>

#include <span>
#include <typeinfo>

/*
server,
client,
engine,
dataCache,
materialSystem,
vstdlib,
vgui2,
vguiMatSurface,
vphysics,
inputSystem,
studioRender,
shaderApiDx9,
serverBrowser
*/

namespace fd
{
struct library_info;
struct csgo_library_info;
struct current_library_info;

class dos_nt
{
    void construct(const LDR_DATA_TABLE_ENTRY* ldrEntry);

  public:
    // base address
    IMAGE_DOS_HEADER* dos;
    IMAGE_NT_HEADERS* nt;

    dos_nt(const LDR_DATA_TABLE_ENTRY* ldrEntry);
    explicit dos_nt(library_info info);

    PVOID base() const;

    std::span<uint8_t>              read() const;
    std::span<IMAGE_SECTION_HEADER> sections() const;

    template <typename T = uint8_t, typename Q>
    T* map(Q obj) const
    {
        const auto dosAddr = reinterpret_cast<uintptr_t>(dos);
        uintptr_t  offset;
        if constexpr (std::is_pointer_v<Q>)
            offset = reinterpret_cast<uintptr_t>(obj);
        else
            offset = static_cast<uintptr_t>(obj);
        return reinterpret_cast<T*>(dosAddr + offset);
    }
};

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

  private:
    pointer entry_;

  public:
    library_info();
    library_info(pointer entry);

    bool is_root() const;
    bool unload() const;

    pointer   get() const;
    pointer   operator->() const;
    reference operator*() const;

    explicit operator bool() const;

    wstring_view path() const;
    wstring_view name() const;

    [[deprecated]] void log_class_info(string_view rawName, const void* addr) const;

    template <class T>
    [[deprecated]] void log_class_info(const T* addr) const
    {
        log_class_info(type_name<T>(), addr);
    }

    void*                 find_export(string_view name, bool notify = true) const;
    IMAGE_SECTION_HEADER* find_section(string_view name, bool notify = true) const;
    void*                 find_signature(string_view sig, bool notify = true) const;

  private:
    void* find_vtable_class(string_view name, bool notify) const;
    void* find_vtable_struct(string_view name, bool notify) const;
    void* find_vtable_unknown(string_view name, bool notify) const;

  public:
    void* find_vtable(string_view name, bool notify = true) const;
    void* find_vtable(const type_info& info, bool notify = true) const;

    template <class T>
    T* find_vtable(const bool notify = true) const
    {
        return static_cast<T*>(find_vtable(typeid(T), notify));
    }
};

library_info find_library(PVOID baseAddress, bool notify = true);
library_info find_library(wstring_view name, bool notify = true);
library_info wait_for_library(wstring_view name, bool notify = true); // todo: delay or cancel

struct csgo_library_info : library_info
{
    using library_info::library_info;

    void* find_interface(string_view name, bool notify = true) const;
    void* find_interface(const void* createInterfaceFn, string_view name, bool notify = true) const;
};

library_info current_library_info(bool notify = true);
void         set_this_library_handle(HMODULE handle);
} // namespace fd