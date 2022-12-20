#include <fd/assert.h>
#include <fd/exception.h>
#include <fd/gui/context_impl.h>
#include <fd/library_info.h>

#include <fd/mem_scanner.h>

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
    : context_(ImGui::GetCurrentContext())
{
    ImGui::GetAllocatorFunctions(&allocator_, &deleter_, &userData_);
}

//--------

keys_pack::keys_pack(const std::initializer_list<value_type> list)
    : basic_keys_pack(list)
{
}

string_view keys_pack::name() const
{
    return name_;
}

static ImGuiKey& operator++(ImGuiKey& key)
{
    ++reinterpret_cast<std::underlying_type_t<ImGuiKey>&>(key);
    return key;
}

static ImGuiKey operator++(ImGuiKey& key, int)
{
    const auto oldKey = key;
    ++key;
    return oldKey;
}

static ImGuiKey operator+(const ImGuiKey l, const ImGuiKey r)
{
    return static_cast<ImGuiKey>(static_cast<std::underlying_type_t<ImGuiKey>>(l) + r);
}

static class : std::array<string_view, ImGuiKey_NamedKey_COUNT>
{
    string_view keys_[ImGuiKey_NamedKey_COUNT];

  public:
    string_view operator()(const ImGuiKey key) const
    {
        return keys_[key - ImGuiKey_NamedKey_BEGIN];
    }

    void fill()
    {
        const auto firstKey = ImGui::GetKeyName(ImGuiKey_NamedKey_BEGIN);
        const auto rng = dos_nt(current_library_info()).read();
        const auto namesBuff = *invoke(xrefs_scanner(rng.data(), rng.size()), reinterpret_cast<uintptr_t>(firstKey));

        keys_[0] = firstKey;

        auto keyOffset = static_cast<ImGuiKey>(1);

        if (namesBuff)
        {
            for (size_t i = keyOffset; i < ImGuiKey_NamedKey_COUNT; ++i)
                keys_[i] = static_cast<const char**>(namesBuff)[i];
        }
        else
        {
            std::ranges::for_each(std::begin(keys_) + keyOffset, std::end(keys_), [&](string_view& str) {
                str = ImGui::GetKeyName(ImGuiKey_NamedKey_BEGIN + keyOffset++);
            });
        }
    }

} _KeyNames;

void keys_pack::update_name()
{
    if (empty())
    {
        name_ = "(UNSET)";
        return;
    }

    name_.clear();

    auto names = *this | std::views::transform(/*ImGui::GetKeyName*/ _KeyNames);
    auto sizes = names | std::views::transform(&string_view::size);

    constexpr auto decorationsBefore = 1; //'('
    constexpr auto decorationsInside = 1; //'+'
    constexpr auto decorationsAfter = 1;  //')'

    const auto decorationsCount = decorationsBefore + (size() - 1) * decorationsInside + decorationsAfter;
    const auto charsCount = std::accumulate(sizes.begin(), sizes.end(), static_cast<size_t>(0));
    name_.reserve(decorationsCount + charsCount);

    name_ += '(';
    std::ranges::for_each(names.begin(), names.end() - 1, [&](auto val) {
        name_.append_range(val);
        name_ += '+';
    });
    name_.append_range(names.back());
    name_ += ')';
}

//--------

template <typename Fn>
class keys_pack_ex : public basic_keys_pack
{
    [[no_unique_address]] Fn fn_;
    bool                     filled_ = false;

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
            for (auto key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
            {
                if (invoke(fn_, key))
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

#define UPDATE_KEYS_GLOBALLY
static keys_pack_ex<pressed_filler> _KeysPressed;
static keys_pack_ex<held_filler>    _KeysHeld;
#ifdef _DEBUG
static bool  _Dummy = false;
static bool& _HotkeysActive = _Dummy;
#endif

#define UNKNOWN_HK_MODE FD_ASSERT_UNREACHABLE("Unknown hotkey mode!")

hotkey::hotkey(hotkey_data&& data)
    : hotkey_data(std::move(data))
{
}

bool hotkey::update(const bool allowOverride)
{
    FD_ASSERT(_HotkeysActive);
#ifdef WORKAROUND_MULTIPLE_KEYS
#ifndef UPDATE_KEYS_GLOBALLY
    _KeysHeld.fill();
#endif
    const basic_keys_pack& currKeys = _KeysHeld;
#else
    const auto& currKeys = [=]() -> basic_keys_pack& {
        switch (hotkey_data::mode)
        {
        case press:
#ifndef UPDATE_KEYS_GLOBALLY
            _KeysPressed.fill();
#endif
            return _KeysPressed;
        case held:
#ifndef UPDATE_KEYS_GLOBALLY
            _KeysHeld.fill();
#endif
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

    keys.assign_range(currKeys);
    keys.update_name();
    return true;
}

void hotkey::mark_unused()
{
    hotkey_data::source = 0u;
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
    hotkey_mode   mode_;

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
        return hk.source == 0u;
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
#ifndef UPDATE_KEYS_GLOBALLY
        if (!buff.fill())
            return;
#endif
        if (hk.keys != buff)
            return;
        invoke(hk.callback);
    };

    const auto selectMode = [](hotkey_data& hk) {
        if (hk.source == 0u)
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
                return (hk.access & access) != 0;
            }),
            selectMode
        );
}

auto hotkeys_storage::create(hotkey&& hk) -> pointer
{
    auto& hotkeyData = static_cast<hotkey_data&>(hk);
    FD_ASSERT(!contains(hk));
    FD_ASSERT(!!hotkeyData.callback);

    std::ranges::sort(hotkeyData.keys);
    hotkeyData.keys.update_name();

    const auto unusedHotkey = find_unused();
    if (unusedHotkey != nullptr)
    {
        *unusedHotkey = std::move(hk);
        return unusedHotkey;
    }
    return &storage_.emplace_back(std::move(hk));
}

bool hotkeys_storage::erase(const hotkey_source source, const hotkey_mode mode)
{
#ifdef _DEBUG
    if (storage_.empty())
        return false;
#endif

    const auto itr = std::ranges::find_if(storage_, hotkey_comparer(source, mode));
    const auto found = itr != storage_.end();
    if (found)
        itr->mark_unused();
    return found;
}

//-------

basic_hotkey* context::find_basic_hotkey(const hotkey_source source, const hotkey_mode mode)
{
    return hotkeys_.find(source, mode);
}

context::~context()
{
    if (find_library(L"d3d9.dll", false))
        ImGui_ImplDX9_Shutdown();
    ImGui::Shutdown();
    std::destroy_at(&_KeysHeld);
    std::destroy_at(&_KeysPressed);
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
    , hotkeysActive_(false)
{
#ifdef _DEBUG
    _HotkeysActive = hotkeysActive_;
#endif

    IMGUI_CHECKVERSION();
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions( //
        [](const size_t size, void*) -> void* {
            return new uint8_t[size];
        },
        [](void* buff, void*) {
            delete[] static_cast<uint8_t*>(buff);
        }
    );
#endif
    ImGui::SetCurrentContext(&context_);
    ImGui::Initialize();
    _KeyNames.fill();

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

    // move to main
    if (!ImGui_ImplDX9_Init(d3d) || !ImGui_ImplWin32_Init(hwnd))
        unload();
}

void context::release_textures()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

#ifdef _DEBUG
// ReSharper disable once CppInconsistentNaming
#define D3D_VALIDATE(_X_) FD_ASSERT(_X_ == D3D_OK)
#else
// ReSharper disable once CppInconsistentNaming
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
    const auto minimized = displaySize.x <= 0 || displaySize.y <= 0;
    if (minimized)
        return;
#endif

    ImGui::NewFrame();
    {
        hotkeysActive_ = focused_ && !context_.InputEventsTrail.empty();
        if (hotkeysActive_)
        {
            _KeysHeld.clear();
            _KeysPressed.clear();
#ifdef UPDATE_KEYS_GLOBALLY
            if (!_KeysHeld.fill() && !_KeysPressed.fill())
                hotkeysActive_ = false;
#endif
        }

        std::ranges::for_each(callbacks_, Invoker);

        if (hotkeysActive_)
        {
            const auto haveVisibleWindow = std::ranges::any_of(context_.WindowsFocusOrder, [](auto wnd) {
                return wnd->Active || wnd->Collapsed;
            });
            hotkeys_.fire(haveVisibleWindow ? hotkey_access::foreground : hotkey_access::background);
        }
    }
    ImGui::Render();

    D3D_VALIDATE(thisPtr->BeginScene());
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    D3D_VALIDATE(thisPtr->EndScene());
}

struct keys_data
{
    HWND   window;
    UINT   message;
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
#if 0
    if (!can_process_keys())
        return process_keys_result::native;
#endif

    const auto&     events = context_.InputEventsQueue;
    const auto      oldEventsCount = events.size();
    const auto      instant = ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0;
    const std::span eventsAdded(events.begin() + oldEventsCount, events.end());

    // update focus
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

    if (context_.IO.AppFocusLost)
        return process_keys_result::native;
    if (instant)
        return process_keys_result::instant;
    if (!focused_ || eventsAdded.empty())
        return process_keys_result::native;
    return process_keys_result::def;
}

void context::store(callback_type&& callback)
{
    callbacks_.emplace_back(std::move(callback));
}

hotkey* context::create_hotkey(hotkey_data&& hk)
{
    return hotkeys_.create(std::move(hk));
}

hotkey* context::find_hotkey(const hotkey_source source, const hotkey_mode mode)
{
    return hotkeys_.find(source, mode);
}

bool context::erase_hotkey(const hotkey_source source, const hotkey_mode mode)
{
    return hotkeys_.erase(source, mode);
}

bool context::update_hotkey(const hotkey_source source, const hotkey_mode mode, const bool allowOverride)
{
    if (!hotkeysActive_)
        return false;
    const auto hk = hotkeys_.find(source, mode);
    return hk != nullptr && hk->update(allowOverride);
}