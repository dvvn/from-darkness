#pragma once
#include "system.h"

namespace fd
{
class native_library_info : public system_library_info
{
  public:
    using system_library_info::system_library_info;

    void *interface(string_view name) const;
};

template <library_tag Tag>
struct native_library;
} // namespace fd