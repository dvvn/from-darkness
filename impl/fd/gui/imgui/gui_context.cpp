#include <fd/object.h>

#include <imgui_impl_dx9.h>
#include <imgui_internal.h>

#include <d3d9.h>

struct gui_context
{
    ImGuiContext ctx;
    ImFontAtlas atlas;

    ~gui_context()
    {
        ImGui::Shutdown();
    }

    gui_context(const gui_context&)            = delete;
    gui_context& operator=(const gui_context&) = delete;

    gui_context()
        : ctx(&atlas)
    {
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(&ctx);
        ImGui::Initialize();
        ctx.SettingsHandlers.clear(); // remove default ini handler
        ctx.IO.IniFilename = nullptr; //

        // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();
    }
};

FD_OBJECT_ATTACH_EX(ImGuiContext*, std::addressof(FD_OBJECT_GET(gui_context)->ctx));
