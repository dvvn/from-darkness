module;

#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <windows.h>

#include <exception>

module fd.hooks.winapi;
import fd.functional.bind;

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;
using namespace hooks;

#if 0

static auto _Find_window()
{
    LONG_PTR wp;

    if (rt_modules::current->is_root())
        EnumWindows(_Wnd_Callback, reinterpret_cast<LPARAM>(&wp));
    else
        wp = GetWindowLongPtrA(FindWindowA("Valve001", nullptr), GWLP_WNDPROC);

    return reinterpret_cast<WNDPROC>(wp);
}

#define ARGS hwnd, msg, wparam, lparam

FD_HOOK(wndproc, _Find_window(), static, LRESULT WINAPI, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const auto input_result = invoke(gui::input_handler, ARGS);
    const auto block_input  = input_result.touched();
    LRESULT ret;
    if (!block_input)
        ret = call_original(ARGS);
    else if (input_result.have_return_value())
        ret = input_result.return_value();
    else
        ret = invoke(DefWindowProcW, ARGS);
    return ret;
}

#endif

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

    TCHAR name[MAX_PATH];
    GetClassName(hwnd, name, MAX_PATH);
    WNDCLASSEX wc;
    if (!GetClassInfoEx(data->handle, name, &wc))
        return TRUE;

    data->wndproc = wc.lpfnWndProc;
    data->hwnd    = hwnd;
    return FALSE;
}

struct window_info
{
    void* wndproc;
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

static window_info _Find_wndproc(PTCHAR name) //"Valve001"
{
    const auto hwnd = FindWindow(name, nullptr);
    const auto lptr = GetWindowLongPtr(hwnd, GWLP_WNDPROC);
    return { reinterpret_cast<void*>(lptr), hwnd };
}

static wndproc* _Wndproc;

template <typename Fn, typename T>
static void _Init(wndproc* self, Fn init_fn, T arg)
{
    const auto [wndproc, hwnd] = _Find_wndproc(arg);
    invoke(init_fn, self, wndproc);
    if (!ImGui_ImplWin32_Init(hwnd))
        std::terminate();
    _Wndproc = self;
}

wndproc::wndproc(HMODULE handle)
{
    _Init(this, bind_back(&wndproc::init, &wndproc::callback), handle);
}

wndproc::wndproc(PTCHAR name)
{
    _Init(this, bind_back(&wndproc::init, &wndproc::callback), name);
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
    _Wndproc = this;
}

string_view wndproc::name() const
{
    return "WinAPI.WndProc";
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

LRESULT WINAPI wndproc::callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
        return 1;
    return invoke(&wndproc::callback, _Wndproc->get_original_method(), window, message, w_param, l_param);
}
