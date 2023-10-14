#include "basic_dx11.h"
//
#include "diagnostics/system_error.h"

#include <imgui_impl_dx11.h>

#pragma comment(lib, "d3d9.lib")

namespace fd
{
basic_dx11_backend::~basic_dx11_backend()
{
    ImGui_ImplDX9_Shutdown();
}

basic_dx11_backend::basic_dx11_backend(IDirect3DDevice9* device)
{
    if (!ImGui_ImplDX9_Init(device))
        throw system_error("Unable to init ImGui_ImplDX9!");
}

void basic_dx11_backend::new_frame()
{
    ImGui_ImplDX9_NewFrame();
}

void basic_dx11_backend::render(ImDrawData* draw_data)
{
    ImGui_ImplDX9_RenderDrawData(draw_data);
}

void basic_dx11_backend::reset()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}
}