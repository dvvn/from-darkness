module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_signature;
export import :find_library;

FDS_RTM_NOTIFICATION(on_signature_found, packed_library_name, std::string_view /*sig*/, void* /*found ptr*/);

uint8_t* find_signature(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig, const bool notify = true);

template <fds::chars_cache Module, fds::chars_cache Sig>
void* find_signature()
{
    static const auto found = [] {
        const auto ldr_entry = fds::find_library<Module>();
        const auto ptr       = find_signature(ldr_entry, Sig, false);
        std::invoke(on_signature_found, Module, Sig, ptr);
        return ptr;
    }();
    return found;
}

export namespace fds
{
    using ::find_signature;
    using ::on_signature_found;
} // namespace fds
