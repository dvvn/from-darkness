module;

#include <d3d9.h>

export module fd.hooks.directx;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct d3d9_reset : impl
    {
        d3d9_reset(function_getter target);
        d3d9_reset(d3d9_reset&& other);

      private:
        void WINAPI callback(D3DPRESENT_PARAMETERS* params);
    };

    struct d3d9_present : impl
    {
        d3d9_present(function_getter target);
        d3d9_present(d3d9_present&& other);

      private:
        struct present_args
        {
            CONST RECT* source_rect;
            CONST RECT* desc_rect;
            HWND dest_window_override;
            CONST RGNDATA* dirty_region;
        };

        HRESULT WINAPI callback(present_args args);
    };
} // namespace fd::hooks
