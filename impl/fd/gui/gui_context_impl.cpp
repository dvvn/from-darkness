module;

#include <fd/assert.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

#include <algorithm>
#include <exception>
#include <vector>

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

//--------

void keys_pack::sort()
{
    if (size() <= 1)
        return;
    std::sort(begin(), end());
    FD_ASSERT(*begin() != *std::next(begin()));
}

bool keys_pack::operator==(const keys_pack& other) const
{
    return std::operator==(*this, other);
}

//--------

context_impl::~context_impl()
{
    if (library_info::_Find(L"d3d9.dll", false))
        ImGui_ImplDX9_Shutdown();

    ImGui::Shutdown();
}

static void _Disable_ini(ImGuiContext* ctx = ImGui::GetCurrentContext())
{
    ctx->SettingsHandlers.clear();
    ctx->IO.IniFilename = nullptr;
}

struct init_data
{
    IDirect3DDevice9* d3d;
    HWND hwnd;
};

context_impl::context_impl(void* data, const bool store_settings)
    : context_(&font_atlas_)
{
    IMGUI_CHECKVERSION();
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
        _Disable_ini(&context_);
    // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    //---

    const auto [d3d, hwnd] = *static_cast<init_data*>(data);
    if (!ImGui_ImplDX9_Init(d3d))
        std::terminate();
    if (!ImGui_ImplWin32_Init(hwnd))
        std::terminate();

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    store([] {
        ImGui::ShowDemoWindow();
    });
#endif
}

void context_impl::release_textures()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void context_impl::render(void* data)
{
    const auto d3d = static_cast<IDirect3DDevice9*>(data);

    // Start the Dear ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // todo: return if minimized

    ImGui::NewFrame();
    {
        for (auto& fn : data_)
            fn();
    }
    ImGui::EndFrame();

    [[maybe_unused]] const auto bg = d3d->BeginScene();
    IM_ASSERT(bg == D3D_OK);
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
    [[maybe_unused]] const auto ed = d3d->EndScene();
    IM_ASSERT(ed == D3D_OK);
}

struct keys_data
{
    HWND window;
    UINT message;
    WPARAM w_param;
    LPARAM l_param;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

// global array to prevent reallocation
static keys_pack _Tmp_keys;

char context_impl::process_keys(void* data)
{
    const auto [wnd, msg, wp, lp] = *static_cast<keys_data*>(data);

    const auto& events      = context_.InputEventsQueue;
    const auto size_before  = events.size();
    const bool done         = ImGui_ImplWin32_WndProcHandler(wnd, msg, wp, lp);
    const auto size_after   = events.size();
    const auto events_added = size_after - size_before;

    if (events_added == 0)
        return done;

    const auto ret = done ? TRUE : CHAR_MAX;

    if (hotkeys_.empty())
        return ret;

    _Tmp_keys.clear();

    const auto events_ed = events.end();
    for (auto itr = events_ed - events_added; itr != events_ed; ++itr)
    {
        ImGuiKey key;
        const auto event_type = itr->Type;
        if (event_type == ImGuiInputEventType_Key) // WIP! see Down variable
        {
            key = itr->Key.Key;
            if (!ImGui::IsNamedKeyOrModKey(key))
                return ret;
        }
        else if (event_type == ImGuiInputEventType_MouseButton) // WIP! see Down variable
        {
            key = ImGui::MouseButtonToKey(itr->MouseButton.Button);
        }
        else
        {
            return ret;
        }

        _Tmp_keys.push_back(key);
    }
    _Tmp_keys.sort();

    for (auto& hk : hotkeys_)
    {
        if (!hk.used)
            continue;
        if (hk.keys != _Tmp_keys)
            continue;
        // WIP
    }

    return ret;
}

void context_impl::store_hotkey(hotkey&& hk, const bool sort_keys)
{
    FD_ASSERT(!hk.keys.empty());
    FD_ASSERT(std::all_of(hk.keys.begin(), hk.keys.end(), ImGui::IsNamedKeyOrModKey));
    if (sort_keys)
    {
        hk.keys.sort();
        hk.keys.shrink_to_fit();
    }

    for (auto& old_hk : hotkeys_)
    {
        if (!old_hk.used)
        {
            old_hk = std::move(hk);
            return;
        }
    }

    hotkeys_.emplace_back(std::move(hk));
}
