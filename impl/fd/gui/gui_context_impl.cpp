module;

#include <fd/assert.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <d3d9.h>

#include <algorithm>
#include <span>
#include <vector>

module fd.gui.context.impl;
import fd.library_info;

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

template <typename Fn>
class keys_pack_ex : public keys_pack
{
    [[no_unique_address]] Fn fn_;

  public:
    keys_pack_ex(const bool instant_fill = false, Fn fn = {})
        : fn_(fn)
    {
    }

    bool fill()
    {
        if (empty())
        {
            using key_t = std::underlying_type_t<ImGuiKey>;

            constexpr auto key_first = ImGuiKey_NamedKey_BEGIN;
            constexpr auto key_last  = ImGuiKey_NamedKey_END;

            for (auto key = key_first; key < key_last; ++reinterpret_cast<key_t&>(key))
            {
                if (fn_(key))
                    push_back(key);
            }
        }

        return !empty();
    }
};

struct pressed_filler
{
    bool operator()(const ImGuiKey key) const
    {
        return ImGui::IsKeyPressed(key, false);
    }
};

using pressed_keys_pack = keys_pack_ex<pressed_filler>;

struct held_filler
{
    bool operator()(const ImGuiKey key) const
    {
        return ImGui::IsKeyDown(key);
    }
};

using held_keys_pack = keys_pack_ex<held_filler>;

#define UNKNOWN_HK_MODE FD_ASSERT_UNREACHABLE("Unknown hotkey mode!")

bool hotkey::update(const bool allow_override)
{
    keys_pack curr_keys;
    switch (mode /*keys.size() <= 1 ? mode : held*/)
    {
    case press:
        curr_keys = pressed_keys_pack(true);
        break;
    case held:
        curr_keys = held_keys_pack(true);
        break;
    default:
        UNKNOWN_HK_MODE;
    }

    if (curr_keys.empty())
        return false;
    if (!allow_override && !keys.empty())
    {
        if (!std::ranges::includes(curr_keys, keys))
            return false;
    }
    keys = std::move(curr_keys);
    return true;
}

//-------

class hotkey_comparer
{
    hotkey_source source_;
    hotkey_mode mode_;

  public:
    hotkey_comparer(hotkey_source source, hotkey_mode mode)
        : source_(source)
        , mode_(mode)
    {
        FD_ASSERT(static_cast<bool>(source));
    }

    bool operator()(const hotkey& hk) const
    {
        return hk.source == source_ && hk.mode == mode_;
    }
};

template <typename It>
static auto _Extract_ptr(It itr, It last_itr) // decltype(&*std::declval<It>())
{
    return itr == last_itr ? nullptr : &*itr;
}

auto hotkeys_storage::find(hotkey_source source, hotkey_mode mode) -> pointer
{
    const auto itr = std::ranges::find_if(storage_, hotkey_comparer(source, mode));
    return _Extract_ptr(itr, storage_.end());
}

auto hotkeys_storage::find(hotkey_source source, hotkey_mode mode) const -> const_pointer
{
    const auto itr = std::ranges::find_if(storage_, hotkey_comparer(source, mode));
    return _Extract_ptr(itr, storage_.end());
}

bool hotkeys_storage::contains(hotkey_source source, hotkey_mode mode) const
{
    return std::ranges::any_of(storage_, hotkey_comparer(source, mode));
}

auto hotkeys_storage::find_unused() -> pointer
{
    const auto itr = std::ranges::find_if(storage_, [](auto& hk) {
        return !hk.source;
    });
    return _Extract_ptr(itr, storage_.end());
}

void hotkeys_storage::fire()
{
    if (storage_.empty())
        return;

    constexpr auto _Fire = [](auto& hk, auto& buff) {
        /*if (!hk.source)
            return;*/
        if (!buff.fill())
            return;
        if (hk.keys != buff)
            return;
        hk.callback();
    };

    std::ranges::for_each(storage_, [pressed = pressed_keys_pack(), held = held_keys_pack()](auto& hk) mutable {
        if (!hk.source)
            return;
        switch (hk.mode)
        {
        case hotkey_mode::press: {
            _Fire(hk, pressed);
            break;
        }
        case hotkey_mode::held: {
            _Fire(hk, held);
            break;
        }
        default: {
            UNKNOWN_HK_MODE;
        }
        }
    });
}

hotkey* hotkeys_storage::create(hotkey_source source, hotkey_mode mode, callback_type callback)
{
    FD_ASSERT(!contains(source, mode));
    FD_ASSERT(static_cast<bool>(callback));

    auto hk = find_unused();
    if (hk)
        hk->keys.clear();
    else
        hk = &storage_.emplace_back();

    hk->source   = source;
    hk->mode     = mode;
    hk->callback = std::move(callback);

    return hk;
}

//-------

void context_impl::fire_hotkeys()
{
    if (!focused_)
        return;
    if (context_.InputEventsTrail.empty())
        return;
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
    hotkeys_.fire();
#ifdef _DEBUG
    ImGui::SetCurrentContext(&context_);
#endif
}

bool context_impl::can_process_keys() const
{
    // are all windows closed?
    // are any overlay visible?
    // are any hotkey exists?

    return true;
}

context_impl::~context_impl()
{
    if (library_info::_Find(L"d3d9.dll", false))
        ImGui_ImplDX9_Shutdown();

    ImGui::Shutdown();
}

static void _Disable_ini_settings(ImGuiContext* ctx = ImGui::GetCurrentContext())
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
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(
        [](const size_t size, void*) -> void* {
            return new uint8_t[size];
        },
        [](void* buff, void*) {
            const auto correct_buff = static_cast<uint8_t*>(buff);
            delete[] correct_buff;
        });
#endif
    ImGui::SetCurrentContext(&context_);

    ImGui::Initialize();

#ifndef _DEBUG
    // todo: disable fallback window
#endif

    if (!store_settings)
        _Disable_ini_settings(&context_);
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

#ifdef _DEBUG
#define D3D_VALIDATE(_X_) FD_ASSERT(_X_ == D3D_OK)
#else
#define D3D_VALIDATE(_X_) _X_
#endif

void context_impl::render(void* data)
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

#ifndef IMGUI_HAS_VIEWPORT
    const auto display_size = context_.IO.DisplaySize;
    const auto minimized    = display_size.x <= 0 || display_size.y <= 0;
    if (minimized)
        return ImGui::EndFrame();
#endif

    std::ranges::for_each(callbacks_, invoker());

    const auto have_visible_window = std::ranges::any_of(context_.WindowsFocusOrder, [](auto wnd) {
        return wnd->Active || wnd->Collapsed;
    });

    if (!have_visible_window)
        fire_hotkeys();

    const auto d3d = static_cast<IDirect3DDevice9*>(data);

    D3D_VALIDATE(d3d->BeginScene());
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    D3D_VALIDATE(d3d->EndScene());
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
    constexpr char ret_default = std::numeric_limits<char>::max();

    if (!can_process_keys())
        return ret_native;

    const auto [wnd, msg, wp, lp] = *static_cast<keys_data*>(data);

    const auto& events          = context_.InputEventsQueue;
    const auto old_events_count = events.size();
    const bool instant          = ImGui_ImplWin32_WndProcHandler(wnd, msg, wp, lp);
    const std::span events_added(events.begin() + old_events_count, events.end());

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
}

void context_impl::store(callback_type callback)
{
    callbacks_.emplace_back(std::move(callback));
}

bool context_impl::create_hotkey(hotkey_source source, hotkey_mode mode, callback_type callback, const bool update)
{
    auto hk = hotkeys_.create(source, mode, callback);
    if (update)
    {
        FD_ASSERT(focused_);
        return hk->update(true);
    }
    return true;
}

bool context_impl::update_hotkey(hotkey_source source, hotkey_mode mode, const bool allow_override)
{
    if (!focused_)
        return false;
    const auto hk = hotkeys_.find(source, mode);
    return hk && hk->update(allow_override);
}

bool context_impl::remove_hotkey(hotkey_source source, hotkey_mode mode)
{
    auto hk = hotkeys_.find(source, mode);
    if (hk)
        hk->source = 0;
    return hk;
}

bool context_impl::contains_hotkey(hotkey_source source, hotkey_mode mode) const
{
    return hotkeys_.contains(source, mode);
}
