#pragma once

#include <imgui_impl_dx11.h>

#include <cassert>

#pragma comment(lib, "d3d11.lib")

namespace fd::gui
{
class basic_dx11_backend
{
  protected:
    ~basic_dx11_backend()
    {
        ImGui_ImplDX11_Shutdown();
    }

    basic_dx11_backend(ID3D11Device* device, ID3D11DeviceContext* device_context)
    {
        auto const init = ImGui_ImplDX11_Init(device, device_context);
        assert(init == true);
    }

  public:
    using draw_data = ImDrawData;

    static void new_frame()
    {
        ImGui_ImplDX11_NewFrame();
    }

    static void render(draw_data* data)
    {
        ImGui_ImplDX11_RenderDrawData(data);
    }

    static void reset()
    {
        ImGui_ImplDX11_InvalidateDeviceObjects();
    }
};
} // namespace fd::gui