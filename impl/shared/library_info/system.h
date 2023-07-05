#pragma once
#include "basic.h"

namespace fd
{
struct basic_pattern;

class system_library_info : public basic_library_info
{
  public:
    using basic_library_info::basic_library_info;

    void *function(string_view name) const;
    void *pattern(basic_pattern const &pattern) const;
    void *vtable(string_view name) const;
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