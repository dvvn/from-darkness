module;

#include <fd/assert.h>

#include <imgui_internal.h>

module fd.gui.context;

using namespace fd::gui;

imgui_backup::~imgui_backup()
{
    ImGui::SetAllocatorFunctions(allocator_, deleter_, user_data_);
    ImGui::SetCurrentContext(context_);
}

imgui_backup::imgui_backup()
{
    context_ = ImGui::GetCurrentContext();
    ImGui::GetAllocatorFunctions(&allocator_, &deleter_, &user_data_);
}

#ifdef _DEBUG
static context* ctx_;
#endif

context::~context()
{
    // if (active_)
    ImGui::Shutdown();
#ifdef _DEBUG
    ctx_ = nullptr;
#endif
}

context::context(const bool store_settings)
    : context_(&font_atlas_)
{
#ifdef _DEBUG
    FD_ASSERT(ctx_ == nullptr, "GUI context aleady set!");
    IMGUI_CHECKVERSION();
    ctx_ = this;
#endif

    ImGui::SetAllocatorFunctions(
        [](const size_t size, void*) -> void* {
            return new uint8_t[size];
        },
        [](void* buff, void*) {
            const auto correct_buff = static_cast<uint8_t*>(buff);
            delete[] correct_buff;
        });
    ImGui::SetCurrentContext(&context_);
    ImGui::Initialize();
    if (!store_settings)
    {
        context_.SettingsHandlers.clear();
        context_.IO.IniFilename = nullptr;
    }
    // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();
}
