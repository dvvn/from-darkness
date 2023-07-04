#pragma once

namespace fd
{
#ifdef _DEBUG
using hook_name = struct string_view;
#else
using hook_name = char const *;
#endif
}