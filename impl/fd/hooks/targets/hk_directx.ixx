module;

#include <d3d9.h>

export module fd.hooks.directx;
import fd.hooks.impl;

export namespace fd::hooks
{
    struct d3d9_reset : impl
    {
        ~d3d9_reset() override;

        d3d9_reset(function_getter target);
        d3d9_reset(d3d9_reset&& other);

        string_view name() const override;

      private:
        void WINAPI callback(D3DPRESENT_PARAMETERS* params);
    };

    struct d3d9_present : impl
    {
        ~d3d9_present() override;

        d3d9_present(function_getter target);
        d3d9_present(d3d9_present&& other);

        string_view name() const override;

      private:
        HRESULT WINAPI callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region);
    };
} // namespace fd::hooks
