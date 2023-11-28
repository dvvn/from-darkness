#pragma once

#include <cstdint>

struct IDXGISwapChain;

namespace fd::native
{
struct dx11_swap_chain
{
    uint8_t pad[0x178];
    IDXGISwapChain* pDXGISwapChain;
};
}