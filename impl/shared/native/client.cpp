#include "client.h"
#include "library_info/native.h"

namespace fd
{
native_client::native_client(native_library_info const info)
{
    construct_at(this, info.interface("VClient"));
}
}