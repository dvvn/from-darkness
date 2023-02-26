#include <fd/netvars/basic_info.h>

namespace fd
{
bool operator<(basic_netvar_info const& l, basic_netvar_info const& r)
{
    return l.offset() < r.offset();
}
} // namespace fd