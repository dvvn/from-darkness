module;

#include <d3d9.h>

export module fd.hooks.directx;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct d3d9_reset : impl, instance<d3d9_reset>
    {
        d3d9_reset(function_getter target);

        void WINAPI callback(D3DPRESENT_PARAMETERS* params) noexcept;
    };

    struct d3d9_present : impl, instance<d3d9_present>
    {
        d3d9_present(function_getter target);

        struct present_args
        {
        };

        HRESULT WINAPI callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region) noexcept;
    };
} // namespace fd::hooks
