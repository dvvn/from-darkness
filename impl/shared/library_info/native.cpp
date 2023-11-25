#include "library_info/native.h"

namespace fd
{
native_library_info literals::operator""_dlln(wchar_t const* name, size_t length)
{
    return {
        {name, length},
        native_library_info::extension_tag::dll
    };
}
} // namespace fd
