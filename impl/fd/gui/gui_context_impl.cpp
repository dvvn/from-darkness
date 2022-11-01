module;

#include <fd/assert.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

#include <algorithm>
#include <exception>
#include <span>
#include <vector>

module fd.gui.context.impl;
import fd.library_info;
import fd.functional.lazy_invoke;
import fd.functional.bind;

using namespace fd;
using namespace gui;

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

constexpr auto _Fill_keys = [](auto fn, keys_pack& keys) {
    if (!keys.empty())
        return;

    using key_t = std::underlying_type_t<ImGuiKey>;

    constexpr auto key_first = ImGuiKey_NamedKey_BEGIN;
    constexpr auto key_last  = ImGuiKey_NamedKey_END;

    for (auto key = key_first; key < key_last; ++reinterpret_cast<key_t&>(key))
    {
        if (fn(key))
            keys.push_back(key);
    }
};

struct pressed_keys_pack : keys_pack
{
    pressed_keys_pack(const bool instant_fill = false)
    {
        if (instant_fill)
            fill();
    }

    void fill()
    {
        _Fill_keys(bind_back(ImGui::IsKeyPressed, false), *this);
    }
};

struct held_keys_pack : keys_pack
{
    held_keys_pack(const bool instant_fill = false)
    {
        if (instant_fill)
            fill();
    }

    void fill()
    {
        _Fill_keys(ImGui::IsKeyDown, *this);
    }
};

#define UNKNOWN_HK_MODE FD_ASSERT_UNREACHABLE("Unknown hotkey mode!")

void context_impl::fire_hotkeys()
{
    if (!focused_ || hotkeys_.empty())
        return;
    if (context_.InputEventsTrail.empty())
        return;

    pressed_keys_pack pressed;
    held_keys_pack held;

    for (auto& hk : hotkeys_)
    {
        if (!hk.used)
            continue;
        switch (hk.mode)
        {
        case hokey_mode::press: {
            pressed.fill();
            if (hk.keys == pressed)
                hk.callback();
            break;
        }
        case hokey_mode::held: {
            held.fill();
            if (hk.keys == held)
                hk.callback();
            break;
        }
        default: {
            UNKNOWN_HK_MODE;
        }
        }
    }
}

bool context_impl::can_process_keys() const
{
    return true;
}

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
    store_callback([] {
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
    auto can_render = false;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if (!minimized())
    {
        fire_hotkeys();
        can_render = !callbacks_.empty();
        if (can_render)
        {
            for (auto& fn : callbacks_)
                fn();
        }
    }
    ImGui::EndFrame();

    if (can_render)
    {
        const auto d3d = static_cast<IDirect3DDevice9*>(data);

        [[maybe_unused]] const auto bg = d3d->BeginScene();
        IM_ASSERT(bg == D3D_OK);
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
        [[maybe_unused]] const auto ed = d3d->EndScene();
        IM_ASSERT(ed == D3D_OK);
    }
}

struct keys_data
{
    HWND window;
    UINT message;
    WPARAM w_param;
    LPARAM l_param;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

char context_impl::process_keys(void* data)
{
    constexpr char ret_instant = TRUE;
    constexpr char ret_native  = FALSE;
    constexpr char ret_default = CHAR_MAX;

    if (!can_process_keys())
        return ret_native;

    const auto [wnd, msg, wp, lp] = *static_cast<keys_data*>(data);

    const auto& events           = context_.InputEventsQueue;
    const auto old_events_count  = events.size();
    const bool instant           = ImGui_ImplWin32_WndProcHandler(wnd, msg, wp, lp);
    const std::span events_added = { events.begin() + old_events_count, events.end() };

    // update focus
    switch (msg)
    {
    case WM_SETFOCUS: {
        focused_ = true;
        break;
    }
    case WM_KILLFOCUS: {
        focused_ = false;
        break;
    }
    }

    if (context_.IO.AppFocusLost)
    {
        FD_ASSERT(!instant);
        return ret_native;
    }
    if (instant)
        return ret_instant;
    return !focused_ || events_added.empty() ? ret_native : ret_default;

#if 0
    if (hotkeys_.empty())
        return ret;

    _Tmp_keys.clear();

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
#endif
}

void context_impl::store_callback(callback_type callback)
{
    callbacks_.emplace_back(std::move(callback));
}

bool context_impl::create_hotkey(void* source, hokey_mode mode, callback_type callback)
{
    hotkey* hk;

    for (auto& old_hk : hotkeys_)
    {
        if (!old_hk.used)
        {
            hk = &old_hk;
            break;
        }
    }

    if (!hk)
        hk = &hotkeys_.emplace_back();

    hk->source   = source;
    hk->mode     = mode;
    hk->callback = callback;

    switch (mode)
    {
    case press:
        hk->keys = pressed_keys_pack(true);
        break;
    case held:
        hk->keys = held_keys_pack(true);
        break;
    default:
        UNKNOWN_HK_MODE;
    }

    // todo: check is already added

    if (hk->keys.empty())
        hk->used = false;

    return hk->used;
}

void context_impl::remove_hotkey(void* source, hokey_mode mode)
{
    FD_ASSERT(source != nullptr);

    for (auto& hk : hotkeys_)
    {
        if (hk.source != source)
            continue;
        if (mode != any)
        {
            if (hk.mode != mode)
                continue;
        }
        hk.used = false;
    }
}

bool context_impl::minimized() const
{
    return context_.IO.DisplaySize.x * context_.IO.DisplaySize.y <= 0;
}
