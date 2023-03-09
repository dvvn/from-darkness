#pragma once

#include <fd/hidden_ptr.h>

#include <windows.h>
#include <winternl.h>

#include <functional>
#include <mutex>
#include <semaphore>
#include <span>
#include <vector>

namespace fd
{
struct library_info
{
    using pointer   = LDR_DATA_TABLE_ENTRY *;
    using reference = LDR_DATA_TABLE_ENTRY &;

  protected:
    pointer entry_;

  public:
    library_info(pointer entry = nullptr);

    bool is_root() const;
    [[deprecated]]
    bool unload() const;

    pointer get() const;
    pointer operator->() const;
    reference operator*() const;

    std::wstring_view path() const;
    std::wstring_view name() const;

    hidden_ptr find_export(std::string_view name) const;
    IMAGE_SECTION_HEADER *find_section(std::string_view name) const;

    hidden_ptr find_signature(std::string_view sig) const;

    hidden_ptr find_vtable(std::string_view name) const;
    hidden_ptr find_vtable(std::type_info const &info) const;

    template <class T>
    T *find_vtable() const
    {
        return find_vtable(typeid(T));
    }
};

bool operator==(library_info info, std::nullptr_t);
bool operator==(library_info info, library_info other);
bool operator==(library_info info, PVOID base_address);
bool operator==(library_info info, std::wstring_view name);
bool operator!(library_info info);

library_info find_library(PVOID base_address);
library_info find_library(std::wstring_view name);

struct csgo_library_info : library_info
{
    using library_info::library_info;

    csgo_library_info(library_info info);

    hidden_ptr find_interface(std::string_view name) const;
    hidden_ptr find_interface(void const *create_interface_fn, std::string_view name) const;
};

library_info current_library_info();
void set_current_library(HMODULE handle);
} // namespace fd