#pragma once

#ifdef _DEBUG
#include <fmt/core.h>

#include <functional>
#endif
#include <cstdint>

// ReSharper disable CppInconsistentNaming

using LDR_DATA_TABLE_ENTRY = struct _LDR_DATA_TABLE_ENTRY;
using IMAGE_DOS_HEADER     = struct _IMAGE_DOS_HEADER;
using IMAGE_SECTION_HEADER = struct _IMAGE_SECTION_HEADER;

#ifdef _WIN64
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS64;
#else
using IMAGE_NT_HEADERS = struct _IMAGE_NT_HEADERS;
#endif

namespace fd
{
using system_library_entry  = LDR_DATA_TABLE_ENTRY const *;
using system_section_header = IMAGE_SECTION_HEADER const *;

using system_cstring     = wchar_t const *;
using system_string_view = struct wstring_view;

struct string_view;

template <typename T>
struct span;

#ifdef _DEBUG
void log(fmt::string_view fmt, fmt::format_args fmt_args, std::ostream *out);
#endif

class system_library
{
    system_library_entry entry_;

  public:
    system_library(system_string_view name);

    system_string_view name() const;
    system_string_view path() const;
    span<uint8_t> memory() const;
    void *function(string_view name) const;
    uint8_t *pattern(string_view pattern) const;
    system_section_header section(string_view name) const;
    void *rtti_descriptor(string_view class_name) const;
    void *vtable(string_view name) const;

#ifdef _DEBUG
    using bound_name = decltype(std::bind(&system_library::name, std::declval<system_library const *>()));

  private:
    fmt::string_view merge_fmt_args(fmt::string_view fmt) const;

  protected:
    template <typename... Args>
    void log(fmt::format_string<Args...> fmt, Args &&...args) const
    {
        fd::log(merge_fmt_args(fmt.get()), fmt::format_args(fmt::make_format_args(args...)), (std::ostream *)nullptr);
    }
#endif
};
} // namespace fd

#ifdef _DEBUG
namespace std
{
template <typename Fn, typename T>
auto bind(Fn &&fn, T &&ptr) requires(
    convertible_to<Fn, decltype(&fd::system_library::name)> && !same_as<decay_t<T>, fd::system_library const *>)
{
    return bind(forward<Fn>(fn), forward_like<T &&>(static_cast<fd::system_library const *>(ptr)));
}
} // namespace std

template <>
struct fmt::formatter<fd::system_library::bound_name> : formatter<string_view>
{
    auto format(fd::system_library::bound_name binder, format_context &ctx) const -> format_context::iterator;
};
#endif