module;

#include <imgui.h>
#include <imgui_internal.h>

#include <optional>

module fd.gui.context;

class gui_context
{
    ImGuiContext context_;
    ImFontAtlas font_atlas_;

    bool active_;
    ImGuiContext* old_context_;

  public:
    ~gui_context()
    {
        if (active_)
            ImGui::Shutdown();
        ImGui::SetCurrentContext(old_context_);
    }

    gui_context(const gui_context&)            = delete;
    gui_context& operator=(const gui_context&) = delete;

    gui_context()
        : context_(&font_atlas_)
    {
        IMGUI_CHECKVERSION();
        old_context_ = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(&context_);
        ImGui::Initialize();
        context_.SettingsHandlers.clear(); // remove default ini handler
        context_.IO.IniFilename = nullptr; //

        // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        active_ = true;
    }

    void reset()
    {
        if (!active_)
            return;
        ImGui::Shutdown();
        active_ = false;
    }
};

static std::optional<gui_context> _Context;

void init_context()
{
    _Context.emplace();
}

void destroy_context()
{
    if (_Context)
        _Context->reset();
}
