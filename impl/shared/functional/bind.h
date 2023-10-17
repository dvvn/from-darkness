#pragma once

#include <functional>

namespace fd
{
#ifdef __cpp_lib_bind_back
using std::bind_back;
#endif
#ifdef __cpp_lib_bind_front
using std::bind_front;
#endif
using std::bind;
}