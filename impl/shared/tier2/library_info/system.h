#pragma once
#include "tier2/library_info/basic.h"

namespace FD_TIER(2)
{
class system_library_info : public basic_library_info
{
  public:
    using basic_library_info::basic_library_info;

    void* function(string_view name) const;
    void* vtable(string_view name) const;
};

inline namespace literals
{
system_library_info operator"" _dll(wchar_t const* name, size_t length);

template <static_string Name>
system_library_info operator"" _dll()
{
    return {Name, system_library_info::extension_tag::dll};
}
} // namespace literals
} // namespace FD_TIER(2)