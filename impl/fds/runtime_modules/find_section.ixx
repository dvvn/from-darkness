module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_section;
export import :find_library;

FDS_RTM_NOTIFICATION(on_section_found, packed_library_name, std::string_view /*section name*/, IMAGE_SECTION_HEADER* /*section ptr*/);

IMAGE_SECTION_HEADER* find_section(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view name, const bool notify = true);

template <fds::chars_cache Module, fds::chars_cache Section>
IMAGE_SECTION_HEADER* find_section()
{
    static const auto found = [] {
        const auto ldr_entry = fds::find_library<Module>();
        const auto ptr       = find_section(ldr_entry, Section, false);
        std::invoke(on_section_found, Module, Section, ptr);
        return ptr;
    }();
    return found;
}

export namespace fds
{
    using ::find_section;
    using ::on_section_found;
} // namespace fds
