#pragma once
#include "native/interface_register.h"
#include "library_info.h"

namespace fd
{
namespace detail
{
class native_library_interface_getter
{
    native::interface_register* root_interface_;

    static native::interface_register* get_root_interface(library_info const* linfo);

  public:
    native_library_interface_getter(library_info const* linfo);
    safe_cast_lazy<void*> find(string_view name) const;
};
} // namespace detail
} // namespace fd
