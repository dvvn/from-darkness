module;

#include <imgui.h>

#include <d3d9.h>

module fd.hooks.directx;
import fd.gui.menu;
import fd.gui.context;

using namespace fd;
using namespace hooks;

d3d9_reset::d3d9_reset(function_getter target)
    : impl("IDirect3DDevice9::Reset")
    , instance(target)
{
}

void WINAPI d3d9_reset::callback(D3DPRESENT_PARAMETERS* params)
{
    gui::context->release_textures();
    call_original(params);
}

//------------

d3d9_present::d3d9_present(function_getter target)
    : impl("IDirect3DDevice9::Present")
    , instance(target)
{
}

HRESULT WINAPI d3d9_present::callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    if (gui::context->begin_frame())
    {
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        ImGui::ShowDemoWindow();
#endif
        gui::menu->render();
        //---
        gui::context->end_frame();
    }
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
