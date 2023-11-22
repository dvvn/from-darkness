#pragma once
#include "library_info/basic.h"

namespace fd
{
class system_library_info : public basic_library_info
{
  public:
    using basic_library_info::basic_library_info;

    void* function(char const* name, size_t length) const;

    template <size_t Length>
    void* function(char const (&name)[Length]) const
    {
        return function(name, Length - 1);
    }

    void* vtable(char const* name, size_t length) const;

    template <size_t Length>
    void* vtable(char const (&name)[Length]) const
    {
        return vtable(name, Length - 1);
    }
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
} // namespace fd