#include <fd/gui/context.h>
#include <fd/gui/menu.h>
#include <fd/hooked/hk_directx.h>

using namespace fd;
using namespace hooked;

d3d9_reset::d3d9_reset(function_getter target)
    : hook_impl("IDirect3DDevice9::Reset")
    , hook_instance(target)
{
}

void WINAPI d3d9_reset::callback(D3DPRESENT_PARAMETERS* params) noexcept
{
    gui::context->release_textures();
    call_original(params);
}

//------------

d3d9_present::d3d9_present(function_getter target)
    : hook_impl("IDirect3DDevice9::Present")
    , hook_instance(target)
{
}

HRESULT WINAPI d3d9_present::callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region) noexcept
{
    gui::context->render(this);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
