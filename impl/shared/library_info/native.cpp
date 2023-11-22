#include "library_info/native.h"
#include "native/interface_register.h"

namespace fd
{
void* native_library_info::interface(char const* name, size_t const length, interface_register const* root)
{
    auto const found = find(root, name, length);
    return found ? found->get() : nullptr;
}

void* native_library_info::interface(char const* name, size_t const length) const
{
    return interface(name, length, root_interface());
}

native_library_info literals::operator""_dlln(wchar_t const* name, size_t length)
{
    return {name, length, native_library_info::extension_tag_dll};
}
} // namespace fd
