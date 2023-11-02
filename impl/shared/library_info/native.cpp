#include "library_info/native.h"
#include "native/interface_register.h"

namespace fd
{
void* native_library_info::interface(char const* name, size_t length, interface_register* root)
{
    interface_register* found;
#ifdef _DEBUG
    found = find_unique(root, nullptr, name, length);
#else
    found = find(root_interface, nullptr, name, length);
#endif
    return found ? found->get() : nullptr;
}

void* native_library_info::interface(char const* name, size_t const length) const
{
    return interface(name, length, root_interface());
}

} // namespace fd
