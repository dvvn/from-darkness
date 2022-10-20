module;

#include <fd/assert.h>

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <windows.h>

#include <exception>

module fd.hooks.winapi;
import fd.functional.bind;
#ifdef IMGUI_DISABLE_DEMO_WINDOWS
import fd.gui.menu;
#endif

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;
using namespace hooks;

struct callback_data
{
    HMODULE handle;
    WNDPROC wndproc = nullptr;
    HWND hwnd;
};

static BOOL CALLBACK _Wnd_Callback(HWND hwnd, LPARAM lparam)
{
    if (!IsWindowVisible(hwnd))
        return TRUE;

    const auto data = reinterpret_cast<callback_data*>(lparam);

    WNDPROC wp = nullptr;

    if (IsWindowUnicode(hwnd))
    {
        wchar_t name[MAX_PATH];
        GetClassNameW(hwnd, name, MAX_PATH);
        WNDCLASSEXW wc;
        if (GetClassInfoExW(data->handle, name, &wc))
            wp = wc.lpfnWndProc;
    }
    else
    {
        char name[MAX_PATH];
        GetClassNameA(hwnd, name, MAX_PATH);
        WNDCLASSEX wc;
        if (GetClassInfoExA(data->handle, name, &wc))
            wp = wc.lpfnWndProc;
    }
    if (!wp)
        return TRUE;

    data->wndproc = wp;
    data->hwnd    = hwnd;
    return FALSE;
}

struct window_info
{
    WNDPROC wndproc;
    HWND hwnd;
};

static window_info _Find_wndproc(HMODULE handle)
{
    callback_data data = { handle };
    EnumWindows(_Wnd_Callback, reinterpret_cast<LPARAM>(&data));
    if (!data.wndproc)
        std::terminate();
    return { data.wndproc, data.hwnd };
}

//"Valve001"

static WNDPROC _Get_wndproc(const HWND hwnd)
{
    const auto lptr = invoke(IsWindowUnicode(hwnd) ? GetWindowLongPtrW : GetWindowLongPtrA, hwnd, GWLP_WNDPROC);
    return reinterpret_cast<WNDPROC>(lptr);
}

static window_info _Find_wndproc(const char* name)
{
    const auto hwnd = FindWindowA(name, nullptr);
    return { _Get_wndproc(hwnd), hwnd };
}

static window_info _Find_wndproc(const wchar_t* name)
{
    const auto hwnd = FindWindowW(name, nullptr);
    return { _Get_wndproc(hwnd), hwnd };
}

static wndproc* _Wndproc;

template <typename Fn, typename T, typename H>
static void _Init(wndproc* self, Fn init_fn, T arg, WNDPROC& def_wp, H&& h)
{
    const auto [wndproc, hwnd] = _Find_wndproc(arg);
    if (!ImGui_ImplWin32_Init(hwnd))
        std::terminate();
    _Wndproc = self;
    def_wp   = IsWindowUnicode(hwnd) ? DefWindowProcW : DefWindowProcA;
    h        = hwnd;
    invoke(init_fn, self, wndproc);
}

#ifdef _DEBUG
#define HWND_ARG this->hwnd_
#else
#define HWND_ARG static_cast<HWND>(nullptr)
#endif

#define INIT(_ARG_) _Init(this, bind_back(&wndproc::init, &wndproc::callback), _ARG_, this->def_, HWND_ARG)

wndproc::wndproc(HMODULE handle)
{
    INIT(handle);
}

wndproc::wndproc(const char* name)
{
    INIT(name);
}

wndproc::wndproc(const wchar_t* name)
{
    INIT(name);
}

wndproc::~wndproc()
{
    // added for logging only
    if (*this)
        impl::disable();
}

wndproc::wndproc(wndproc&& other)
    : impl(std::move(other))
{
    *static_cast<wndproc_data*>(this) = other;
    _Wndproc                          = this;
}

string_view wndproc::name() const
{
    return "WinAPI.WndProc";
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

LRESULT WINAPI wndproc::callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
#define ARGS window, message, w_param, l_param

    FD_ASSERT(window == _Wndproc->hwnd_);

#ifdef IMGUI_DISABLE_DEMO_WINDOWS
    if (!gui::menu->visuble())
        return invoke(&wndproc::callback, _Wndproc->get_original_method(), ARGS);
#endif

    const auto ctx         = ImGui::GetCurrentContext();
    const auto size_before = ctx->InputEventsQueue.size();

    if (ImGui_ImplWin32_WndProcHandler(ARGS))
        return TRUE;

    const auto size_after = ctx->InputEventsQueue.size();
    return size_before == size_after ? invoke(&wndproc::callback, _Wndproc->get_original_method(), ARGS) : invoke(_Wndproc->def_, ARGS);
}
