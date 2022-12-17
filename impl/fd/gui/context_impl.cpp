#include <fd/assert.h>
#include <fd/gui/context_impl.h>
#include <fd/library_info.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <span>

using namespace fd;
using namespace gui;

// workaround, because imgui have poor muliple keys handling
#define WORKAROUND_MULTIPLE_KEYS

imgui_backup::~imgui_backup()
{
    ImGui::SetAllocatorFunctions(allocator_, deleter_, userData_);
    ImGui::SetCurrentContext(context_);
}

imgui_backup::imgui_backup()
{
    context_ = ImGui::GetCurrentContext();
    ImGui::GetAllocatorFunctions(&allocator_, &deleter_, &userData_);
}

//--------

string_view keys_pack::name() const
{
    return name_;
}

void keys_pack::update_name()
{
    if (empty())
    {
        name_ = "(UNSET)";
        return;
    }

    name_.clear();

    auto names = *this | std::views::transform(ImGui::GetKeyName);
    write_string(name_, '(');
    std::ranges::for_each(names.begin(), names.end() - 1, [&](auto val) {
        write_string(name_, val, '+');
    });
    write_string(name_, names.back(), ')');
}

//--------

template <typename Fn>
class keys_pack_ex : public basic_keys_pack
{
    [[no_unique_address]] Fn fn_;
    bool filled_ = false;

  public:
    keys_pack_ex(const bool instantFill = false, Fn fn = {})
        : fn_(fn)
    {
        if (instantFill)
            fill();
    }

    bool fill()
    {
        if (!filled_)
        {
            using key_t = std::underlying_type_t<ImGuiKey>;

            constexpr auto keyFirst = ImGuiKey_NamedKey_BEGIN;
            constexpr auto keyLast  = ImGuiKey_NamedKey_END;

            for (auto key = keyFirst; key < keyLast; ++reinterpret_cast<key_t&>(key))
            {
                if (fn_(key))
                    basic_keys_pack::push_back(key);
            }
            filled_ = true;
        }

        return !empty();
    }

    void clear()
    {
#ifdef _DEBUG
        if (!filled_)
            return;
#endif
        filled_ = false;
        basic_keys_pack::clear();
    }
};

struct pressed_filler
{
    bool operator()(const ImGuiKey key) const
    {
        // return ImGui::IsKeyReleased(key);
        return ImGui::IsKeyPressed(key, false);
    }
};

struct held_filler
{
    bool operator()(const ImGuiKey key) const
    {
        return ImGui::IsKeyDown(key);
    }
};

static keys_pack_ex<pressed_filler> _KeysPressed;
static keys_pack_ex<held_filler> _KeysHeld;

#define UNKNOWN_HK_MODE FD_ASSERT_UNREACHABLE("Unknown hotkey mode!")

hotkey::hotkey(hotkey_data&& data)
    : hotkey_data(std::move(data))
{
}

bool hotkey::update(const bool allowOverride)
{
#ifdef WORKAROUND_MULTIPLE_KEYS
    _KeysHeld.fill();
    const auto& currKeys = _KeysHeld;
#else
    const auto& currKeys = [=]() -> basic_keys_pack& {
        switch (hotkey_data::mode)
        {
        case press:
            _KeysPressed.fill();
            return _KeysPressed;
        case held:
            _KeysHeld.fill();
            return _KeysHeld;
        default:
            UNKNOWN_HK_MODE;
        }
    }();
#endif

    if (currKeys.empty())
        return false;
    if (!allowOverride && !keys.empty() && !std::ranges::includes(currKeys, keys))
        return false;

    keys = currKeys;
    keys.update_name();
    return true;
}

hotkey_source hotkey::source() const
{
    return hotkey_data::source;
}

hotkey_mode hotkey::mode() const
{
    return hotkey_data::mode;
}

hotkey_access hotkey::access() const
{
    return hotkey_data::access;
}

string_view hotkey::name() const
{
    return hotkey_data::keys.name();
}

void hotkey::callback() const
{
    invoke(hotkey_data::callback);
}

//-------

class hotkey_comparer
{
    hotkey_source source_;
    hotkey_mode mode_;

  public:
    hotkey_comparer(const hotkey_source source, const hotkey_mode mode)
        : source_(source)
        , mode_(mode)
    {
        FD_ASSERT(static_cast<bool>(source));
    }

    bool operator()(const hotkey_data& hk) const
    {
        return hk.source == source_ && hk.mode == mode_;
    }
};

template <typename It>
static auto _extract_ptr(It itr, It lastItr) // -> decltype(&*std::declval<It>())
{
    return itr == lastItr ? nullptr : &*itr;
}

auto hotkeys_storage::find(const hotkey_source source, const hotkey_mode mode) -> pointer
{
    const auto itr = std::ranges::find_if(storage_, hotkey_comparer(source, mode));
    return _extract_ptr(itr, storage_.end());
}

auto hotkeys_storage::find(const hotkey_source source, const hotkey_mode mode) const -> const_pointer
{
    const auto itr = std::ranges::find_if(storage_, hotkey_comparer(source, mode));
    return _extract_ptr(itr, storage_.end());
}

bool hotkeys_storage::contains(const hotkey_source source, const hotkey_mode mode) const
{
    return std::ranges::any_of(storage_, hotkey_comparer(source, mode));
}

bool hotkeys_storage::contains(const hotkey_data& other) const
{
    return contains(other.source, other.mode);
}

auto hotkeys_storage::find_unused() -> pointer
{
    const auto itr = std::ranges::find_if(storage_, [](const hotkey_data& hk) {
        return !hk.source;
    });
    return _extract_ptr(itr, storage_.end());
}

void hotkeys_storage::fire(const hotkey_access access)
{
    if (storage_.empty())
        return;

    constexpr auto doCall = [](hotkey_data& hk, auto& buff) {
        /*if (!hk.source)
            return;*/
        if (!buff.fill())
            return;
        if (hk.keys != buff)
            return;
        invoke(hk.callback);
    };

    const auto selectMode = [](hotkey_data& hk) mutable {
        if (!hk.source)
            return;
#ifdef WORKAROUND_MULTIPLE_KEYS
        if (hk.keys.size() >= 2)
            return invoke(doCall, hk, _KeysHeld);
#endif
        switch (hk.mode)
        {
        case hotkey_mode::press: {
            invoke(doCall, hk, _KeysPressed);
            break;
        }
        case hotkey_mode::held: {
            invoke(doCall, hk, _KeysHeld);
            break;
        }
        default: {
            UNKNOWN_HK_MODE;
        }
        }
    };

    if (access == hotkey_access::any)
        std::ranges::for_each(storage_, selectMode);
    else
        std::ranges::for_each(
            storage_ | std::views::filter([=](const hotkey_data& hk) {
                return hk.access & access;
            }),
            selectMode
        );
}

void hotkeys_storage::create(hotkey&& hkTmp)
{
    [[maybe_unused]] const hotkey_data& hkData = hkTmp;
    FD_ASSERT(!contains(hkData));
    FD_ASSERT(static_cast<bool>(hkData.callback));

    auto hkNew = find_unused();
    if (!hkNew)
        hkNew = &storage_.emplace_back();

    *hkNew = std::move(hkTmp);
}

//-------

void context::fire_hotkeys(const hotkey_access access)
{
    if (!focused_)
        return;
    if (context_.InputEventsTrail.empty())
        return;
    hotkeys_.fire(access);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool context::can_process_keys() const
{
    // are all windows closed?
    // are any overlay visible?
    // are any hotkey exists?

    return true;
}

basic_hotkey* context::find_basic_hotkey(hotkey_source source, hotkey_mode mode)
{
    return hotkeys_.find(source, mode);
}

context::~context()
{
    if (find_library(L"d3d9.dll", false))
        ImGui_ImplDX9_Shutdown();

    ImGui::Shutdown();
}

static void _disable_ini_settings(ImGuiContext* ctx = ImGui::GetCurrentContext())
{
    ctx->SettingsHandlers.clear();
    ctx->IO.IniFilename = nullptr;
}

static void _correct_io(ImGuiIO& io = ImGui::GetIO())
{
}

context::context(IDirect3DDevice9* d3d, const HWND hwnd, const bool storeSettings)
    : context_(&fontAtlas_)
    , focused_(GetForegroundWindow() == hwnd)
{
    IMGUI_CHECKVERSION();
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(
        [](const size_t size, void*) -> void* {
            return new uint8_t[size];
        },
        [](void* buff, void*) {
            const auto correctBuff = static_cast<uint8_t*>(buff);
            delete[] correctBuff;
        }
    );
#endif
    ImGui::SetCurrentContext(&context_);

    ImGui::Initialize();

#ifndef _DEBUG
    // todo: disable fallback window
#endif

    if (!storeSettings)
        _disable_ini_settings(&context_);
    _correct_io(context_.IO);
    // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    //---

    if (!ImGui_ImplDX9_Init(d3d) || !ImGui_ImplWin32_Init(hwnd))
        invoke(std::get_terminate());

    // #ifndef IMGUI_DISABLE_DEMO_WINDOWS
    //     store([] {
    //         ImGui::ShowDemoWindow();
    //     });
    // #endif
}

void context::release_textures()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

#ifdef _DEBUG
#define D3D_VALIDATE(_X_) FD_ASSERT(_X_ == D3D_OK)
#else
#define D3D_VALIDATE(_X_) _X_
#endif

void context::render(void* data)
{
    render(static_cast<IDirect3DDevice9*>(data));
}

void context::render(IDirect3DDevice9* thisPtr)
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    const auto displaySize = context_.IO.DisplaySize;
    const auto minimized   = displaySize.x <= 0 || displaySize.y <= 0;
    if (minimized)
        return;
#endif

    ImGui::NewFrame();
    {
        _KeysHeld.clear();
        _KeysPressed.clear();

        std::ranges::for_each(callbacks_, Invoker);

        const auto haveVisibleWindow = std::ranges::any_of(context_.WindowsFocusOrder, [](auto wnd) {
            return wnd->Active || wnd->Collapsed;
        });

        fire_hotkeys(haveVisibleWindow ? hotkey_access::foreground : hotkey_access::background);
    }
    ImGui::Render();

    D3D_VALIDATE(thisPtr->BeginScene());
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    D3D_VALIDATE(thisPtr->EndScene());
}

struct keys_data
{
    HWND window;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
};

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

process_keys_result context::process_keys(void* data)
{
    const auto kd = static_cast<keys_data*>(data);
    return process_keys(kd->window, kd->message, kd->wParam, kd->lParam);
}

process_keys_result context::process_keys(const HWND window, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    if (!can_process_keys())
        return process_keys_result::native;

    const auto& events        = context_.InputEventsQueue;
    const auto oldEventsCount = events.size();
    const bool instant        = ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam);
    const std::span eventsAdded(events.begin() + oldEventsCount, events.end());

    // update focus
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (message)
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

    process_keys_result ret;
    if (context_.IO.AppFocusLost)
    {
        FD_ASSERT(!instant);
        ret = process_keys_result::native;
    }
    else if (instant)
        ret = process_keys_result::instant;
    else if (!focused_ || eventsAdded.empty())
        ret = process_keys_result::native;
    else
        ret = process_keys_result::def;
    return ret;
}

void context::store(callback_type&& callback)
{
    callbacks_.emplace_back(std::move(callback));
}

bool context::create_hotkey(hotkey_data&& hkTmp, const bool update)
{
    hotkey hk(std::move(hkTmp));
    std::ranges::sort(hk.keys);

    if (!update)
        hk.keys.update_name();
    else
    {
        FD_ASSERT(focused_);
        if (hk.keys.empty())
            hk.update(true);
        else if (!hk.update(false))
            return false;
    }
    hotkeys_.create(std::move(hk));
    return true;
}

bool context::update_hotkey(const hotkey_source source, const hotkey_mode mode, const bool allowOverride)
{
    if (!focused_)
        return false;
    const auto hk = hotkeys_.find(source, mode);
    return hk && hk->update(allowOverride);
}

bool context::remove_hotkey(const hotkey_source source, const hotkey_mode mode)
{
    hotkey_data* hk = hotkeys_.find(source, mode);
    if (hk)
        hk->source = 0;
    return hk;
}

const hotkey* context::find_hotkey(const hotkey_source source, const hotkey_mode mode) const
{
    return hotkeys_.find(source, mode);
}