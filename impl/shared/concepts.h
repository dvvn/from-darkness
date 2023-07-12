#pragma once

namespace fd
{
template <class T/*, unsigned = __builtin_LINE()*/>
concept forwarded = !requires { sizeof(T); };
} // namespace fd