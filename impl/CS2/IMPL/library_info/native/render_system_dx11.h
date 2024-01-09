#pragma once
#include "algorithm/address.h"
#include "library_info/literals.h"
#include "library_info/pattern_getter.h"
#include "native/DXGI_swap_chain.h"
#include "pattern/make.h"

#include <cassert>

namespace fd
{
using render_system_dx11_dll = named_library_info<"rendersystemdx11">;

namespace detail
{
template <>
class library_object_getter<render_system_dx11_dll>
{
    library_pattern_getter pattern_;

  public:
    library_object_getter(library_info const* linfo)
        : pattern_{linfo}
    {
    }

    IDXGISwapChain* DXGI_swap_chain() const
    {
        auto const addr = pattern_.find("66 0F 7F 05 ? ? ? ? 66 0F 7F 0D ? ? ? ? 48 89 35"_pat);
        assert(addr != nullptr); // nullptr only for range style
        auto const abs_addr = resolve_relative_address(addr, 0x4, 0x8);
        assert(abs_addr != nullptr);
        auto const native_swap_chain = **static_cast<native::DXGI_swap_chain***>(abs_addr);
        assert(native_swap_chain != nullptr);

        return native_swap_chain->pDXGISwapChain;
    }
};

template <size_t I>
auto get(library_object_getter<render_system_dx11_dll> const& getter)
{
    if constexpr (I == 0)
        return getter.DXGI_swap_chain();
}
} // namespace detail
} // namespace fd