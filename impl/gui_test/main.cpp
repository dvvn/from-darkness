#include "backend.h"

#include <fd/assert_impl.h>
#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_impl.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>

#include <imgui.h>

using namespace fd;

class hooks_storage final : public hook_global_callback
{
    std::vector<basic_hook*> hooks_;

    void construct(basic_hook* caller) override
    {
        hooks_.push_back(caller);
    }

    void destroy(const basic_hook* caller, bool unhooked) override
    {
        // std::ranges stuck here
        *std::find(hooks_.begin(), hooks_.end(), caller) = nullptr;
    }

  public:
    bool enable() const
    {
        return std::ranges::all_of(hooks_, &basic_hook::enable);
    }
};

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

#ifdef _DEBUG
    default_assert_handler assertCallback;
    AssertHandler = &assertCallback;
#endif

    system_console sysConsole;

    default_logs_handler logsCallback;
    Logger = &logsCallback;

    logsCallback.add([&](auto msg) {
        sysConsole.write(msg);
    });

#ifdef _DEBUG
    assertCallback.add([&](auto& adata) {
        sysConsole.write(parse_assert_data(adata));
    });
#endif

    gui::menu menu;
    gui::context guiCtx(backend.d3d, backend.hwnd, false);
    guiCtx.create_hotkey({ __LINE__, gui::hotkey_mode::press, bind_front(&gui::menu::toggle, &menu), gui::hotkey_access::any, { ImGuiKey_End } });
    guiCtx.store([&] {
        menu.render();
    });

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    bool imGuiDemoToggle = true;
    guiCtx.create_hotkey({ __LINE__,
                           gui::hotkey_mode::press,
                           [&] {
                               imGuiDemoToggle = !imGuiDemoToggle;
                           },
                           gui::hotkey_access::any,
                           { ImGuiKey::ImGuiKey_Home } });
    guiCtx.store([&] {
        if (imGuiDemoToggle)
            ImGui::ShowDemoWindow();
    });
#endif

    gui::tab_bar testTabBar("test");
    gui::tab testTab("test2");
    testTab.store(bind_front(ImGui::Text, "Hello"));

    testTabBar.store(testTab);
    menu.store(testTabBar);

    hooks_storage allHooks;
    HookGlobalCallback = &allHooks;

    hook_callback hkWndProc(backend.info.lpfnWndProc);
    hkWndProc.set_name("WinAPI.WndProc");
    hkWndProc.add([&](auto&, auto& ret, bool& interrupt, auto... args) {
        const auto val = guiCtx.process_keys(args...);
        if (val == TRUE)
            ret.emplace(TRUE);
        else if (val == FALSE)
            interrupt = true;
    });
    hkWndProc.add([&](auto&, auto& ret, bool, auto... args) {
        ret.emplace(DefWindowProc(args...));
    });

    hook_callback hkDirectx9Reset(&IDirect3DDevice9::Reset, { backend.d3d, 16 });
    hkDirectx9Reset.set_name("IDirect3DDevice9::Reset");
    hkDirectx9Reset.add([&](auto&&...) {
        guiCtx.release_textures();
    });

    hook_callback hkDirectx9Present(&IDirect3DDevice9::Present, { backend.d3d, 17 });
    hkDirectx9Present.set_name("IDirect3DDevice9::Present");
    hkDirectx9Present.add([&](auto&, auto&, bool, auto thisPtr, auto...) {
        guiCtx.render(thisPtr);
    });

    if (allHooks.enable())
    {
        backend.run();
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}