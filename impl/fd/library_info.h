#pragma once

#include <fd/hidden_ptr.h>

#include <windows.h>
#include <winternl.h>

#include <mutex>
#include <semaphore>
#include <vector>

namespace fd
{
struct library_info
{
    using pointer   = LDR_DATA_TABLE_ENTRY*;
    using reference = LDR_DATA_TABLE_ENTRY&;

  protected:
    pointer entry_;

  public:
    library_info(pointer entry = nullptr);

    bool is_root() const;
    [[deprecated]]
    bool unload() const;

    pointer   get() const;
    pointer   operator->() const;
    reference operator*() const;

    std::wstring_view path() const;
    std::wstring_view name() const;

    hidden_ptr            find_export(std::string_view name) const;
    IMAGE_SECTION_HEADER* find_section(std::string_view name) const;

    hidden_ptr find_signature(std::string_view sig) const;

    hidden_ptr find_vtable(std::string_view name) const;
    hidden_ptr find_vtable(std::type_info const& info) const;

    template <class T>
    T* find_vtable() const
    {
        return find_vtable(typeid(T));
    }
};

bool operator==(library_info info, std::nullptr_t);
bool operator==(library_info info, library_info other);
bool operator==(library_info info, PVOID baseAddress);
bool operator==(library_info info, std::wstring_view name);
bool operator!(library_info info);

library_info find_library(PVOID baseAddress);
library_info find_library(std::wstring_view name);

struct csgo_library_info : library_info
{
    using library_info::library_info;

    csgo_library_info(library_info info);

    hidden_ptr find_interface(std::string_view name) const;
    hidden_ptr find_interface(void const* createInterfaceFn, std::string_view name) const;
};

library_info current_library_info();
void         set_current_library(HMODULE handle);

class library_info_cache
{
    struct cached_data
    {
        std::wstring_view     name;
        std::binary_semaphore sem;

        LDR_DATA_TABLE_ENTRY* value;

        cached_data(std::wstring_view name);
        cached_data(LDR_DATA_TABLE_ENTRY* value, std::wstring_view name = {});
    };

    mutable std::mutex                        mtx_;
    std::vector<std::unique_ptr<cached_data>> cache_;
    PVOID                                     cookie_;

  public:
    ~library_info_cache();
    library_info_cache();

    void store(PVOID baseAddress, std::wstring_view name);
    void remove(PVOID baseAddress, std::wstring_view name);

    library_info get(PVOID baseAddress) const;      // return null if not found
    library_info get(std::wstring_view name) const; // return null if not found
    library_info get(std::wstring_view name);       // wait if not found
};
} // namespace fd