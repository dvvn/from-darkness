#pragma once

namespace fd
{
template <class T>
concept forwarded = !requires { sizeof(T); };
} // namespace fd