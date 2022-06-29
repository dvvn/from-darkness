module;

#include <fd/callback.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fd.rt_modules:find_vtable;
// import fd.type_name;

FD_CALLBACK(on_vtable_found, const LDR_DATA_TABLE_ENTRY* /* library name */, std::string_view /*vtable name*/, void* /*vtable ptr*/);

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

/* template <typename T>
T* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const bool notify = true)
{
    static_assert(std::is_class_v<T>, "Unable to get the class name");
    const auto found = find_vtable(ldr_entry, fd::type_name<T>(), notify);
    return static_cast<T*>(found);
} */

export namespace fd
{
    using ::find_vtable;
    using ::on_vtable_found;
} // namespace fd
