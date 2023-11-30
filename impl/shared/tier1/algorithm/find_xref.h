#pragma once

#include "tier0/algorithm/find.h"
#include "tier1/memory/xref.h"

namespace FD_TIER(1)
{
template <typename It, bool Owned, typename Callback = find_callback_gap>
auto find(It first, It const last, xref<Owned> const& xr, Callback callback = {})
{
    return find<It, typename xref<Owned>::iterator, Callback>(first, last, xr.begin(), xr.end(), std::ref(callback));
}
}