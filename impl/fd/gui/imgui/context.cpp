

#include <fd/object.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

import fd.application_info;

struct gui_context
{
    ImGuiContext ctx_;

  public:
    ImGuiContext* operator&()
    {
        return &ctx_;
    }

    ~gui_context()
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::Shutdown();
    }

    gui_context()
        : ctx_(&FD_OBJECT_GET(ImFontAtlas))
    {
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(&ctx_);
        ImGui::Initialize();
        ctx_.SettingsHandlers.clear(); // remove default ini handler
        ctx_.IO.IniFilename = nullptr; //

        // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(fd::app_info->root_window.handle);
        ImGui_ImplDX9_Init(&FD_OBJECT_GET(IDirect3DDevice9));
    }
};

FD_OBJECT_ATTACH(ImGuiContext*, gui_context);
