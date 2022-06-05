module;

#include <fds/runtime_modules/notification_fwd.h>
#include <fds/runtime_modules/run_once.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_export;
export import :find_library;
import fds.type_name;

template <typename FnT>
constexpr std::string_view get_type_name()
{
    return std::is_function_v<FnT> ? fds::type_name<FnT>() : "unknown";
}

FDS_RTM_NOTIFICATION(on_export_found, packed_library_name, std::string_view /*export name*/, void* /*export ptr*/, std::string_view /*export type*/);

void* find_export(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

template <typename FnT, fds::chars_cache Module, fds::chars_cache Export>
FnT find_export()
{
    static const auto found = [] {
        FDS_RTM_RUN_ONCE(Module, Export);
        const auto ldr_entry = fds::find_library<Module>();
        const auto ptr       = find_export(ldr_entry, Export, false);
        std::invoke(on_export_found, Module, Export, ptr, get_type_name<FnT>);
        return ptr;
    }();
    return found;
}

export namespace fds
{
    using ::find_export;
    using ::on_export_found;
} // namespace fds
