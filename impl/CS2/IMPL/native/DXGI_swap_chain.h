#pragma once

#include <cstdint>

struct IDXGISwapChain;

namespace fd::native::inline cs2
{
struct DXGI_swap_chain
{
    uint8_t pad[0x178];
    IDXGISwapChain* pDXGISwapChain;
};
}