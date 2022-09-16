module;

#include <imgui.h>
#include <imgui_internal.h>

#include <optional>

module fd.gui.context;

class imgui_backup
{
    ImGuiMemAllocFunc allocator_;
    ImGuiMemFreeFunc deleter_;
    void* user_data_;
    ImGuiContext* context_;

  public:
    ~imgui_backup()
    {
        ImGui::SetAllocatorFunctions(allocator_, deleter_, user_data_);
        ImGui::SetCurrentContext(context_);
    }

    imgui_backup()
    {
        context_ = ImGui::GetCurrentContext();
        ImGui::GetAllocatorFunctions(&allocator_, &deleter_, &user_data_);
    }

    imgui_backup(const imgui_backup&)            = delete;
    imgui_backup& operator=(const imgui_backup&) = delete;
};

class gui_context
{
    imgui_backup backup_;
    ImGuiContext context_;
    ImFontAtlas font_atlas_;
    bool active_;

  public:
    ~gui_context()
    {
        if (active_)
            ImGui::Shutdown();
    }

    gui_context()
        : context_(&font_atlas_)
    {
        IMGUI_CHECKVERSION();

        ImGui::SetAllocatorFunctions(
            [](const size_t size, void*) -> void* {
                return new uint8_t[size];
            },
            [](void* buff, void*) {
                auto correct_buff = static_cast<uint8_t*>(buff);
                delete[] correct_buff;
            });
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
        if (active_)
        {
            ImGui::Shutdown();
            active_ = false;
        }
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
