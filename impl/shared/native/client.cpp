#include "client.h"
#include "library_info/native.h"
#include "string/view.h"

namespace fd
{
native_client::native_client(native_library_info info)
    : __vtable(info.interface("VClient"))
{
}

}