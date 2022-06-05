module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_csgo_interface;
export import :find_export;

FDS_RTM_NOTIFICATION(on_csgo_interface_found, packed_library_name, std::string_view /*interface name*/, void* /*interface ptr*/);

void* find_csgo_interface(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);
void* find_csgo_interface(const void* create_interface_fn, const std::string_view name, const bool notify = true);

template <fds::chars_cache Module, fds::chars_cache Interface>
void* find_csgo_interface()
{
    static const auto found = [] {
        const auto create_interface = fds::find_export<void*, Module, "CreateInterface">();
        const auto ptr              = find_csgo_interface(create_interface, Interface, false);
        std::invoke(on_csgo_interface_found, Module, Interface, ptr);
        return ptr;
    }();
    return found;
}

export namespace fds
{
    using ::find_csgo_interface;
    using ::on_csgo_interface_found;
} // namespace fds
