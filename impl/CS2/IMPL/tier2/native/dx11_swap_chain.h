#pragma once

#include "tier2/core.h"

#include <cstdint>

struct IDXGISwapChain;

namespace FD_TIER2(native, cs2)
{
struct dx11_swap_chain
{
    uint8_t pad[0x178];
    IDXGISwapChain* pDXGISwapChain;
};
}