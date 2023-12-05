#pragma once

#include "library_info.h"

namespace fd
{
class native_library_info : public library_info
{
    native::interface_register* root_interface() const;

  protected:
    class basic_interface_getter
    {
      protected:
        native_library_info const* linfo_;

      public:
        basic_interface_getter(native_library_info const* linfo)
            : linfo_{linfo}
        {
        }
    };

    struct interface_string : string_view
    {
        constexpr interface_string(char const* name, size_t length)
            : string_view(name, length)
        {
        }
    };

    static constexpr interface_string operator"" _ifc(char const* name, size_t length)
    {
        return {name, length};
    }

  public:
    using library_info::library_info;

    void* find(interface_string name) const;
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