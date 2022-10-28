module;

#include <fd/assert.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

#include <exception>

module fd.gui.context.impl;
import fd.library_info;

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
static void* ctx_;
#endif

context_impl::~context_impl()
{
    if (library_info::_Find(L"d3d9.dll", false))
        ImGui_ImplDX9_Shutdown();

    ImGui::Shutdown();
#ifdef _DEBUG
    ctx_ = nullptr;
#endif
}

static void _Disable_ini(ImGuiContext* ctx = ImGui::GetCurrentContext())
{
    ctx->SettingsHandlers.clear();
    ctx->IO.IniFilename = nullptr;
}

context_impl::context_impl(const backend_type backend, const bool store_settings)
    : context_(&font_atlas_)
    , backend_(backend)
{
#ifdef _DEBUG
    FD_ASSERT(ctx_ == nullptr, "GUI context_impl aleady set!");
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
    if (!ImGui_ImplDX9_Init(backend))
        std::terminate();
    if (!store_settings)
        _Disable_ini(&context_);
    // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();
}

void context_impl::release_textures()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

bool context_impl::begin_frame()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // todo: return if minimized

    ImGui::NewFrame();

    return true;
}

void context_impl::end_frame()
{
    ImGui::EndFrame();

    [[maybe_unused]] const auto bg = backend_->BeginScene();
    IM_ASSERT(bg == D3D_OK);
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
    [[maybe_unused]] const auto ed = backend_->EndScene();
    IM_ASSERT(ed == D3D_OK);
}