#include <fd/assert.h>
#include <fd/exception.h>
#include <fd/gui/context_impl.h>
#include <fd/library_info.h>
// #include <fd/mem_scanner.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <numeric>

// workaround, because imgui have poor muliple keys handling
// #define WORKAROUND_MULTIPLE_KEYS

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace fd::gui
{
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

#if 0

keys_pack::keys_pack(const std::initializer_list<value_type> list)
    : std::vector<value_type>(list)
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

static class
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
            for (; keyOffset < ImGuiKey_NamedKey_COUNT; ++keyOffset)
                keys_[keyOffset] = static_cast<const char**>(namesBuff)[keyOffset];
        }
        else
        {
            for (; keyOffset < ImGuiKey_NamedKey_COUNT; ++keyOffset)
                keys_[keyOffset] = ImGui::GetKeyName(ImGuiKey_NamedKey_BEGIN + keyOffset);
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

template <typename Fn>
class keys_pack_ex : public keys_pack
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
                    keys_pack::push_back(key);
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
        keys_pack::clear();
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

template <class I>
class hotkey_info_comparer
{
    const I& iView_;

  public:
    hotkey_info_comparer(const I& info)
        : iView_(info)
    {
    }

    template <class I1>
    bool operator()(const I1& info) const
    {
        return (info == iView_);
    }

    template <class I1, typename X>
    bool operator()(const std::pair<I1, X>& data) const
    {
        return (data.first == iView_);
    }
};

bool hotkey_info::update(bool allowOverride)
{
#ifdef WORKAROUND_MULTIPLE_KEYS
#ifndef UPDATE_KEYS_GLOBALLY
    _KeysHeld.fill();
#endif
    const keys_pack& currKeys = _KeysHeld;
#else
    const auto& currKeys = [=]() -> keys_pack& {
        switch (mode)
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

hotkey_data& hotkey::operator[](hotkey_info&& info)
{
    /*std::ranges::sort(info.keys);
    auto itr = std::ranges::find_if(storage_, hotkey_info_comparer(info));
    if (itr == storage_.end())
    {
        info.keys.update_name();
        itr = storage_.emplace(storage_.end(), std::move(info), hotkey_data());
    }
    return itr->second;*/
    return *(hotkey_data*)nullptr;
}

void hotkey::store(hotkey_info&& info, hotkey_data&& data)
{
    FD_ASSERT(!!data.callback);
    std::ranges::sort(info.keys);
    FD_ASSERT(!contains(info));
    info.keys.update_name();
    storage_.emplace_back(std::move(info), std::move(data));
}

const hotkey_data& hotkey::operator[](const hotkey_info_view& info) const
{
    return std::ranges::find_if(storage_, hotkey_info_comparer(info))->second;
}

bool hotkey::contains(const hotkey_info_view& info) const
{
    return std::ranges::any_of(storage_, hotkey_info_comparer(info));
}

bool hotkey::update(const bool allowOverride)
{
    FD_ASSERT(_HotkeysActive);
    const auto updated = std::ranges::count_if(storage_ | std::views::keys, bind_back(&hotkey_info::update, allowOverride)) > 0;
    update_name();
    return updated;
}

bool hotkey::update(hotkey_mode mode, bool allowOverride)
{
    FD_ASSERT(_HotkeysActive);
    const auto filterFn = [=](const hotkey_info& info) {
        return info.mode == mode;
    };
    const auto updated = std::ranges::count_if(storage_ | std::views::keys | std::views::filter(filterFn), bind_back(&hotkey_info::update, allowOverride)) > 0;
    update_name();
    return updated;
}

static std::vector<string_view> _HotkeyNameBuff;

static auto _skip_empty_keys(const hotkey_info& info)
{
    return !info.keys.empty();
}

void hotkey::update_name()
{
    constexpr auto getName = [](const hotkey_info& info) {
        return info.keys.name();
    };

    _HotkeyNameBuff.assign_range(storage_ | std::views::keys | std::views::filter(_skip_empty_keys) | std::views::transform(getName));
    if (_HotkeyNameBuff.empty())
    {
        mergedName_ = "(UNSET)";
        return;
    }

    auto sizes = _HotkeyNameBuff | std::views::transform(&string_view::size);

    const auto delimnersCount = (_HotkeyNameBuff.size() - 1) * 2;
    const auto charsCount = std::accumulate(sizes.begin(), sizes.end(), static_cast<size_t>(0));
    mergedName_.reserve(delimnersCount + charsCount);

    std::ranges::for_each(_HotkeyNameBuff.begin(), _HotkeyNameBuff.end() - 1, [&](auto val) {
        mergedName_.append_range(val);
        mergedName_.append(", ", 2);
    });
    mergedName_.append_range(_HotkeyNameBuff.back());
}

void hotkey::fire(hotkey_access access)
{
#ifdef _DEBUG
    if (storage_.empty())
        return;
#endif

    constexpr auto doCall = [](auto& keys, auto& callback, auto& buff) {

#ifndef UPDATE_KEYS_GLOBALLY
        if (!buff.fill())
            return;
#endif
        if (keys != buff)
            return;
        invoke(callback);
    };

    const auto selectMode = [](auto& val) {
        auto& hk = (val).first;
        auto& data = (val).second;
#ifdef WORKAROUND_MULTIPLE_KEYS
        if (hk.keys.size() >= 2)
            return invoke(doCall, (hk.keys), (data.callback), _KeysHeld);
#endif
        switch (hk.mode)
        {
        case hotkey_mode::press: {
            invoke(doCall, (hk.keys), (data.callback), _KeysPressed);
            break;
        }
        case hotkey_mode::held: {
            invoke(doCall, (hk.keys), (data.callback), _KeysHeld);
            break;
        }
        default: {
            UNKNOWN_HK_MODE;
        }
        }
    };

    if (access == hotkey_access::any)
        (void)std::ranges::for_each(storage_, selectMode);
    else
        std::ranges::for_each(
            storage_ | std::views::filter([=](auto& hk) {
                return (hk.first.access & access) != 0;
            }),
            selectMode
        );
}

string_view hotkey::name() const
{
    return mergedName_;
}

void hotkey::callback() const
{
    for (auto& [info, data] : storage_)
    {
        if (_skip_empty_keys(info))
            continue;
        invoke(data.callback);
    }
}

void hotkeys_storage::fire(const hotkey_access access)
{
#ifdef _DEBUG
    if (empty())
        return;
#endif

    for (auto&& [_, data] : *this)
    {
        data.fire(access);
    }
}

#endif

context_impl::~context_impl()
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

context_impl::context_impl()
    : context_(&fontAtlas_)
    , focused_(false)
#ifdef FD_HAVE_HOTKEY
    , hotkeysActive_(false)
#endif
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&context_);
}

void context_impl::init(bool storeSettings)
{
#if defined(_DEBUG) && defined(FD_HAVE_HOTKEY)
    _HotkeysActive = hotkeysActive_;
#endif

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions( //
        [](const size_t size, void*) -> void* {
            return operator new(size);
        },
        [](void* buff, void*) {
            operator delete(buff);
        }
    );
#endif
    ImGui::Initialize();
#ifdef FD_HAVE_HOTKEY
    _KeyNames.fill();
#endif
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
}

void context_impl::init(IDirect3DDevice9* d3d)
{
    if (!ImGui_ImplDX9_Init(d3d))
        unload();
}

void context_impl::init(HWND hwnd)
{
    focused_ = (GetForegroundWindow() == hwnd);
    if (!ImGui_ImplWin32_Init(hwnd))
        unload();
}

void context_impl::release_textures()
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

// ReSharper disable once CppMemberFunctionMayBeConst
bool context_impl::begin_frame()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    const auto displaySize = context_.IO.DisplaySize;
    const auto minimized   = displaySize.x <= 0 || displaySize.y <= 0;
    if (minimized)
        return false;
#endif

    ImGui::NewFrame();

#ifdef FD_HAVE_HOTKEY
    hotkeysActive_ = focused_ && !context_.InputEventsTrail.empty();
    if (hotkeysActive_)
    {
        _KeysHeld.clear();
        _KeysPressed.clear();
#ifdef UPDATE_KEYS_GLOBALLY
        if ((static_cast<int>(_KeysHeld.fill()) | static_cast<int>(_KeysPressed.fill())) == 0)
            hotkeysActive_ = false;
#endif
    }
#endif

    return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void context_impl::end_frame(IDirect3DDevice9* thisPtr)
{
#ifdef FD_HAVE_HOTKEY
    if (hotkeysActive_)
    {
        const auto haveVisibleWindow = std::ranges::any_of(context_.WindowsFocusOrder, [](auto wnd) {
            return wnd->Active || wnd->Collapsed;
        });
        hotkeys_.fire(haveVisibleWindow ? hotkey_access::foreground : hotkey_access::background);
    }
#endif

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

process_keys_result context_impl::process_keys(void* data)
{
    const auto kd = static_cast<keys_data*>(data);
    return process_keys(kd->window, kd->message, kd->wParam, kd->lParam);
}

process_keys_result context_impl::process_keys(const HWND window, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
#if 0
    if (!can_process_keys())
        return process_keys_result::native;
#endif
    const range_view events(context_.InputEventsQueue);
    const auto       oldEventsCount = _size(events);
    const auto       instant        = ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0;
    const range_view eventsAdded(oldEventsCount, events);

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
    if (!focused_ || _empty(eventsAdded))
        return process_keys_result::native;
    return process_keys_result::def;
}

#ifdef FD_HAVE_HOTKEY

void context_impl::create_hotkey(hotkey_source source, hotkey_info&& info, hotkey_data&& data)
{
    FD_ASSERT(!hotkeys_.contains(source));
    hotkeys_[source].store(std::move(info), std::move(data));
}

hotkey& context_impl::get_hotkey(const hotkey_source source)
{
#ifdef _DEBUG
    return hotkeys_.at(source);
#else
    return **hotkeys_.offset_to(source); // skip throw
#endif
}

// bool context_impl::erase_hotkey(const hotkey_source source, const hotkey_mode mode)
//{
//     return hotkeys_.erase(source, mode);
// }

bool context_impl::update_hotkey(const hotkey_source source, const hotkey_mode mode, const bool allowOverride)
{
    if (!hotkeysActive_)
        return false;
    const auto hk = hotkeys_.offset_to(source);
    if (hk == hotkeys_.end())
        return false;
    return (*hk).second.update(mode, allowOverride);
}

basic_hotkey* context_impl::find_hotkey(const hotkey_source source)
{
    const auto itr = hotkeys_.offset_to(source);
    return itr == hotkeys_.end() ? nullptr : &(*itr).second;
}

#endif
}