module;

#include <fds/runtime_modules/notification_fwd.h>

#include <windows.h>
#include <winternl.h>

#include <string_view>

export module fds.rt_modules:find_library;
export import fds.chars_cache;

#ifndef __cpp_lib_string_contains
#define contains(_X_) find(_X_) != std::wstring_view::npos
#endif

FDS_RTM_NOTIFICATION(on_library_found, std::wstring_view, LDR_DATA_TABLE_ENTRY*);

LDR_DATA_TABLE_ENTRY* find_library_impl(const std::wstring_view name, const bool check_whole_path);
LDR_DATA_TABLE_ENTRY* find_current_library();

template <typename... Ts>
consteval bool check_whole_path(const std::basic_string_view<Ts...> name)
{
    return name.contains(':');
}

template <fds::chars_cache Name>
auto find_library()
{
    static const auto found = find_library_impl(Name, check_whole_path(Name.view()));
    return found;
}

class packed_library_name
{
    union
    {
        std::wstring_view           name_;
        const LDR_DATA_TABLE_ENTRY* ldr_entry_;
    };

    uint8_t idx_;

  public:
    packed_library_name(const std::wstring_view name);
    packed_library_name(const LDR_DATA_TABLE_ENTRY* ldr_entry);

    template <typename T, size_t S>
    packed_library_name(const fds::chars_cache<T, S>& cache)
        : packed_library_name(cache.view())
    {
    }

    operator std::wstring_view() const;

    std::wstring_view unpack() const;
};

export namespace fds
{
    using ::find_current_library;
    using ::find_library;
    using ::on_library_found;
    using ::packed_library_name;
} // namespace fds
