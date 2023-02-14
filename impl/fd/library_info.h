#pragma once

#include <fd/dll_notification.h>
#include <fd/string.h>
#include <fd/type_name.h>

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
    bool unload() const;

    pointer   get() const;
    pointer   operator->() const;
    reference operator*() const;

    wstring_view path() const;
    wstring_view name() const;

    [[deprecated]] void log_class_info(string_view rawName, const void* addr) const;

    template <class T>
    [[deprecated]] void log_class_info(const T* addr) const
    {
        log_class_info(type_name<T>(), addr);
    }

    void*                 find_export(string_view name) const;
    IMAGE_SECTION_HEADER* find_section(string_view name) const;

    void* find_signature(string_view sig) const;

  private:
    void* find_vtable_class(string_view name) const;
    void* find_vtable_struct(string_view name) const;
    void* find_vtable_unknown(string_view name) const;

  public:
    void* find_vtable(string_view name) const;
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
bool operator==(library_info info, wstring_view name);
bool operator!(library_info info);

library_info find_library(PVOID baseAddress);
library_info find_library(wstring_view name);

struct csgo_library_info : library_info
{
    using library_info::library_info;

    csgo_library_info(library_info info);

    void* find_interface(string_view name) const;
    void* find_interface(const void* createInterfaceFn, string_view name) const;
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
    wstring_view          name;
    std::binary_semaphore sem;

    _delayed_library_info(wstring_view name)
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

    void store(PVOID baseAddress, wstring_view name);
    void remove(PVOID baseAddress, wstring name);

    library_info get(PVOID baseAddress) const; // return null if not found
    library_info get(wstring_view name) const; // return null if not found
    library_info get(wstring_view name);       // wait if not found

    void destroy();
};

} // namespace fd