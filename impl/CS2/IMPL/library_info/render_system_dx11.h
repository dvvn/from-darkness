#pragma once
#include "algorithm/address.h"
#include "library_info/pattern.h"
#include "native/DXGI_swap_chain.h"
#include "pattern/make.h"

#include <cassert>

namespace fd
{
template <>
struct detail::library_pattern_getter<struct render_system_dx11_library_info> : library_pattern_getter<>
{
    IDXGISwapChain* DXGI_swap_chain() const
    {
        auto const addr = find("66 0F 7F 05 ? ? ? ? 66 0F 7F 0D ? ? ? ? 48 89 35"_pat);
        assert(addr != nullptr); // nullptr only for range style
        auto const abs_addr = resolve_relative_address(addr, 0x4, 0x8);
        assert(abs_addr != nullptr);
        auto const native_swap_chain = **static_cast<native::DXGI_swap_chain***>(abs_addr);
        assert(native_swap_chain != nullptr);

        return native_swap_chain->pDXGISwapChain;
    }
};

struct render_system_dx11_library_info : native_library_info
{
    render_system_dx11_library_info()
        : native_library_info{L"rendersystemdx11.dll"}
    {
    }

    detail::library_pattern_getter<render_system_dx11_library_info> pattern() const
    {
        return {this};
    }
};
} // namespace fd