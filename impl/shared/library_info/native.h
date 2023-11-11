#pragma once
#include "system.h"

#undef interface

namespace fd
{
inline namespace native
{
class interface_register;
}

struct native_library_info : system_library_info
{
    using system_library_info::system_library_info;

    interface_register* root_interface() const;

    static void* interface(char const* name, size_t length, interface_register* root);

    template <size_t Length>
    static void* interface(char const (&name)[Length], interface_register* root_interface)
    {
        return interface(name, Length - 1, root_interface);
    }

    void* interface(char const* name, size_t length) const;

    template <size_t Length>
    void* interface(char const (&name)[Length]) const
    {
        return interface(name, Length - 1);
    }
};
} // namespace fd