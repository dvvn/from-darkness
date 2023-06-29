﻿#pragma once
#include "basic.h"

namespace fd
{
struct basic_pattern;

class system_library_info : public basic_library_info
{
  public:
    using basic_library_info::basic_library_info;

    void *function(string_view name) const;
    [[deprecated]]
    void *pattern(string_view pattern) const;
    void *pattern(basic_pattern const&pattern)const;
    void *vtable(string_view name) const;
};

template <library_tag Tag>
struct system_library;
} // namespace fd