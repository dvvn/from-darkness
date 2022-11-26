#pragma once

#include <fd/hook_impl.h>

#include <d3d9.h>

namespace fd::hooked
{
    struct d3d9_reset : hook_impl, hook_instance<d3d9_reset>
    {
        d3d9_reset(function_getter target);

        void WINAPI callback(D3DPRESENT_PARAMETERS* params) noexcept;
    };

    struct d3d9_present : hook_impl, hook_instance<d3d9_present>
    {
        d3d9_present(function_getter target);

        HRESULT WINAPI callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region) noexcept;
    };
} // namespace fd::hooked
