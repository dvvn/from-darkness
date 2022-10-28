module;

#include <imgui.h>

#include <d3d9.h>

module fd.hooks.directx;
import fd.functional.invoke;
import fd.gui.menu;
import fd.gui.context;

using namespace fd;
using namespace hooks;

static d3d9_reset* _D3d9_reset;

d3d9_reset::~d3d9_reset()
{
    // added for logging only
    if (*this)
        this->disable();
}

d3d9_reset::d3d9_reset(function_getter target)
{
    this->init(target, &d3d9_reset::callback);
    _D3d9_reset = this;
}

d3d9_reset::d3d9_reset(d3d9_reset&& other)
    : impl(std::move(other))
{
    _D3d9_reset = this;
}

string_view d3d9_reset::name() const
{
    return "IDirect3DDevice9::Reset";
}

void WINAPI d3d9_reset::callback(D3DPRESENT_PARAMETERS* params)
{
    gui::context->release_textures();
    invoke(&d3d9_reset::callback, _D3d9_reset->get_original_method(), this, params);
}

//------------

static d3d9_present* _D3d9_present;

d3d9_present::~d3d9_present()
{
    // added for logging only
    if (*this)
        this->disable();
}

d3d9_present::d3d9_present(function_getter target)
{
    this->init(target, &d3d9_present::callback);
    _D3d9_present = this;
}

d3d9_present::d3d9_present(d3d9_present&& other)
    : impl(std::move(other))
{
    _D3d9_present = this;
}

string_view d3d9_present::name() const
{
    return "IDirect3DDevice9::Present";
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
    return invoke(&d3d9_present::callback, _D3d9_present->get_original_method(), this, source_rect, desc_rect, dest_window_override, dirty_region);
}
