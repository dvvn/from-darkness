
#include <fd/object.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

import fd.gui.basic_render_interface;
import fd.gui.menu;

using namespace fd;
using namespace gui;

struct render_interface_impl final : basic_render_interface
{
    ~render_interface_impl() override
    {
        ImGui_ImplDX9_Shutdown();
    }

    void release_textures() override
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    bool operator()() override
    {
        auto d3d = &FD_OBJECT_GET(IDirect3DDevice9);

        static const auto once = [=] {
            (void)*FD_OBJECT_GET(ImGuiContext*);
            return ImGui_ImplDX9_Init(d3d);
        }();

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        // todo: return if minimized

        ImGui::NewFrame();
        {
            invoke(menu);
        }
        ImGui::EndFrame();

        const auto bg = d3d->BeginScene();
        IM_ASSERT(bg == D3D_OK);
#ifndef _DEBUG
        if (bg != D3D_OK)
            return false;
#endif

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        const auto ed = d3d->EndScene();
        IM_ASSERT(ed == D3D_OK);
#ifndef _DEBUG
        if (ed != D3D_OK)
            return false;
#endif

        return true;
    }
};

FD_OBJECT_BIND_TYPE(render_interface, render_interface_impl);
