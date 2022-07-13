
#include <fd/object.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

import fd.gui.basic_render_interface;

using namespace fd::gui;

struct render_interface_impl final : basic_render_interface
{
    void release_textures() override
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    bool operator()() override
    {
        auto& d3d = *FD_OBJECT_GET(IDirect3DDevice9);

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        // todo: return if minimized

        ImGui::NewFrame();
        {
#ifdef _DEBUG
            ImGui::ShowDemoWindow();
#endif
        }
        ImGui::EndFrame();

        const auto bg = d3d.BeginScene();
        IM_ASSERT(bg == D3D_OK);
#ifndef _DEBUG
        if (bg != D3D_OK)
            return false;
#endif

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        const auto ed = d3d.EndScene();
        IM_ASSERT(ed == D3D_OK);
#ifndef _DEBUG
        if (ed != D3D_OK)
            return false;
#endif

        return true;
    }
};

FD_OBJECT_BIND_TYPE(render_interface, render_interface_impl);
