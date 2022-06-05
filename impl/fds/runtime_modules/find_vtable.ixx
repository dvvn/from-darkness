module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_vtable;
export import :find_library;
import fds.type_name;

FDS_RTM_NOTIFICATION(on_vtable_found, packed_library_name, std::string_view /*vtable name*/, void* /*vtable ptr*/);

template <typename T>
constexpr std::string_view get_type_name()
{
    return std::is_class_v<T> ? fds::type_name<T>() : "unknown";
}

void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

template <fds::chars_cache Module, typename T>
T* find_vtable()
{
    static_assert(std::is_class_v<T>, "Unable to get the class name");

    static const auto found = [] {
        constexpr auto class_name = get_type_name<T>();
        const auto     ldr_entry  = fds::find_library<Module>();
        const auto     ptr        = find_vtable(ldr_entry, class_name, false);
        std::invoke(on_vtable_found, Module, class_name, ptr);
        return ptr;
    }();
    return static_cast<T*>(found);
}

template <fds::chars_cache Module, fds::chars_cache Class>
void* find_vtable()
{
    static const auto found = [] {
        const auto ldr_entry = fds::find_library<Module>();
        const auto ptr       = find_vtable(ldr_entry, Class, false);
        std::invoke(on_vtable_found, Module, Class, ptr);
        return ptr;
    }();
    return found;
}

export namespace fds
{
    using ::find_vtable;
    using ::on_vtable_found;
} // namespace fds
