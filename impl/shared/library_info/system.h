#pragma once
#include "basic.h"

namespace fd
{
struct basic_pattern;

class system_library_info : public basic_library_info
{
  public:
    using basic_library_info::basic_library_info;

    void *pattern(basic_pattern const &pattern) const;

    void *function(char const *name, size_t length) const;

    template <size_t Length>
    void *function(char const (&name)[Length]) const
    {
        return function(name, Length - 1);
    }

    void *vtable(char const *name, size_t length) const;

    template <size_t Length>
    void *vtable(char const (&name)[Length]) const
    {
        return vtable(name, Length - 1);
    }
};

template <library_tag Tag>
struct system_library : system_library_info
{
    system_library()
        : system_library_info(Tag)
    {
    }

    system_library(system_library_info info)
        : system_library_info(info)
    {
    }
};
} // namespace fd