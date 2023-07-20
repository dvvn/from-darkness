#include "engine.h"
#include "library_info/native.h"

namespace fd
{
native_engine::native_engine(native_library_info const info)
{
    construct_at(this, info.interface("VEngineClient"));
}
}