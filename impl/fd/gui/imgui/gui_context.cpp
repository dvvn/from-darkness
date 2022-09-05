#include <fd/object.h>

#include <imgui_impl_dx9.h>
#include <imgui_internal.h>

#include <d3d9.h>

struct gui_context
{
    ImGuiContext ctx_;
    ImFontAtlas atlas_;

  public:
    ImGuiContext* operator&()
    {
        return &ctx_;
    }

    ~gui_context()
    {
        ImGui::Shutdown();
    }

    gui_context()
        : ctx_(&atlas_)
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
    }
};

FD_OBJECT_ATTACH(ImGuiContext*, gui_context);
