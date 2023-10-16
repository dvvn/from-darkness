#include "basic_dx11.h"
//
#include "diagnostics/system_error.h"

#include <imgui_impl_dx11.h>

#pragma comment(lib, "d3d11.lib")

namespace fd
{
basic_dx11_backend::~basic_dx11_backend()
{
    ImGui_ImplDX11_Shutdown();
}

basic_dx11_backend::basic_dx11_backend(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
    if (!ImGui_ImplDX11_Init(device, device_context))
        throw system_error("Unable to init ImGui_ImplDX11!");
}

void basic_dx11_backend::new_frame()
{
    ImGui_ImplDX11_NewFrame();
}

void basic_dx11_backend::render(ImDrawData* draw_data)
{
    ImGui_ImplDX11_RenderDrawData(draw_data);
}

void basic_dx11_backend::reset()
{
    ImGui_ImplDX11_InvalidateDeviceObjects();
}
}