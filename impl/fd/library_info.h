#pragma once

#include <fd/dll_notification.h>

#include <windows.h>
#include <winternl.h>

#include <list>
#include <mutex>
#include <optional>
#include <semaphore>
#include <vector>

namespace fd
{
struct library_info;
struct csgo_library_info;
struct current_library_info;

struct library_info
{
    using pointer   = const LDR_DATA_TABLE_ENTRY*;
    using reference = const LDR_DATA_TABLE_ENTRY&;

  private:
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

    void*                 find_export(std::string_view name) const;
    IMAGE_SECTION_HEADER* find_section(std::string_view name) const;

    void* find_signature(std::string_view sig) const;

  private:
    void* find_vtable_class(std::string_view name) const;
    void* find_vtable_struct(std::string_view name) const;
    void* find_vtable_unknown(std::string_view name) const;

  public:
    void* find_vtable(std::string_view name) const;
    void* find_vtable(const type_info& info) const;

    template <class T>
    T* find_vtable() const
    {
        return static_cast<T*>(find_vtable(typeid(T)));
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

    void* find_interface(std::string_view name) const;
    void* find_interface(const void* createInterfaceFn, std::string_view name) const;
};

library_info current_library_info();
void         set_current_library(HMODULE handle);

struct _dll_notification_funcs
{
    LdrRegisterDllNotification   reg;
    LdrUnregisterDllNotification unreg;
};

#if 0
template <class T>
class _thread_safe_storage_accesser
{
    T*          storage_;
    std::mutex* mtx_;

  public:
    _thread_safe_storage_accesser(const _thread_safe_storage_accesser&)            = delete;
    _thread_safe_storage_accesser& operator=(const _thread_safe_storage_accesser&) = delete;

    ~_thread_safe_storage_accesser()
    {
        mtx_->unlock();
    }

    _thread_safe_storage_accesser(T* storage, std::mutex* mtx)
        : storage_(storage)
        , mtx_(mtx)
    {
        mtx->lock();
    }

    T* operator->() const
    {
        return storage_;
    }

    T& operator*() const
    {
        return *storage_;
    }
};

template <class T>
class _thread_safe_storage
{
    T                  storage_;
    mutable std::mutex mtx_;

  public:
    _thread_safe_storage_accesser<T> get()
    {
        return { storage_, &mtx_ };
    }

    _thread_safe_storage_accesser<const T> get() const
    {
        return { storage_, &mtx_ };
    }
};
#endif

struct _delayed_library_info
{
    std::wstring_view     name;
    std::binary_semaphore sem;

    _delayed_library_info(std::wstring_view name)
        : name(name)
        , sem(0)
    {
    }
};

class library_info_cache
{
    mutable std::mutex mtx_;

    std::vector<library_info>        cache_;
    std::list<_delayed_library_info> delayed_;

    PVOID cookie_;

    inline static std::optional<_dll_notification_funcs> notif_;

    void release_delayed();

  public:
    ~library_info_cache();
    library_info_cache();

    void store(PVOID baseAddress, std::wstring_view name);
    void remove(PVOID baseAddress, std::wstring name);

    library_info get(PVOID baseAddress) const;      // return null if not found
    library_info get(std::wstring_view name) const; // return null if not found
    library_info get(std::wstring_view name);       // wait if not found

    void destroy();
};

} // namespace fd