#pragma once

#include "library_info.h"

namespace fd
{
class native_library_info : public library_info
{
  protected:
    class basic_interface_getter : public basic_object_getter<native_library_info>
    {
        using basic_object_getter::linfo_;

        native::interface_register* root_interface() const;

      protected:
        void* find(string_view name) const;
    };

  public:
    using library_info::library_info;
};

inline namespace literals
{
#ifdef _DEBUG
inline native_library_info operator"" _dlln(wchar_t const* name, size_t length)
{
    return {
        wstring_view{name, length},
        library_info::extension::dll
    };
}
#else
template <static_wstring Name>
native_library_info operator"" _dlln()
{
    return {Name + library_info::extension::dll};
}
#endif
template <static_string Name>
native_library_info operator"" _dlln()
{
    return {Name + library_info::extension::dll};
}
} // namespace literals
}