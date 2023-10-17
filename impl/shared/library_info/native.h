#pragma once
#include "system.h"

#undef interface

namespace fd
{
namespace detail
{
template <typename T>
concept native_interface_have_name = requires { T::name; };
}

// todo: nove into details
struct native_library_info : system_library_info
{
    using system_library_info::system_library_info;

    void* interface(char const* name, size_t length) const;

    template <size_t Length>
    void* interface(char const (&name)[Length]) const
    {
        return interface(name, Length - 1);
    }
};
} // namespace fd