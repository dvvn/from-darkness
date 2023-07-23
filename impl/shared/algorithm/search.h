#pragma once

namespace fd
{
template <typename Ret, typename... Args>
class basic_function;
using search_stop_token = basic_function<bool, void *>;

void *search(void *begin, void const *end, basic_pattern const &pattern, search_stop_token const &token);
void *search(void *begin, void const *end, basic_xref const &xref, search_stop_token const &token);
void *search(void *begin, void const *end, void const *from, void const *to, search_stop_token const &token);
} // namespace fd