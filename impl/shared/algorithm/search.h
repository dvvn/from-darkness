#pragma once

namespace fd
{
struct basic_search_stop_token;

void *search(void *begin, void const *end, basic_pattern const &pattern, basic_search_stop_token const &token);
void *search(void *begin, void const *end, basic_xref const &xref, basic_search_stop_token const &token);
void *search(void *begin, void const *end, void const *from, void const *to, basic_search_stop_token const &token);

} // namespace fd
