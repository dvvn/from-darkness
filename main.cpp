// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "misc/freetype/imgui_freetype.h"

//#include <d3d9.h>
//#define DIRECTINPUT_VERSION 0x0800
//#include <dinput.h>
//#include <tchar.h>

//#pragma comment(lib,"Lib/x86/d3d9.def")
//#pragma comment(lib,"Lib/x86/d3dx9.def")
//#ifdef WIN32
//#pragma comment(lib,"freetype32.lib")
//#else
//#pragma comment(lib,"freetype64.lib")
//#endif

//#include "widgets/game 2048.h"
//#include "widgets/folder browser.h"

#include "main.h"

#include "utl/mem/modules/modules.h"
#include "utl/winapi/lib_load.h"
#include <iostream>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/find_if.hpp>

using namespace utl::winapi;

#include "imgui_dx9.h"

ImGui::imgui_render_helper helper;

#include "minhook_cpp/hook_utils.h"

#include <boost/property_tree/json_parser.hpp>

class target_class
{
public :
    void __stdcall func( )
    {
        (void)this;
        std::cout << "original\n";
    }
};
auto target = target_class( );
class detour_class: public decltype(minhook::detect_hook_holder(&target_class::func))
{
public:
    detour_class( )
    {
        this->before_hook.target_func = std::make_unique<minhook::method_info_member>(&target_class::func);
        this->after_hook.enable = true;
    }

    void __stdcall callback( ) override
    {
        std::cout << "hooked\n";
    }
};

// Main code
int main(int, char**)
{
    //auto target = target_class( );
    //auto a=std::make_unique<minhook::function_info_member_virtual>(nullptr, 1);
    //auto detour = detour_class( );
    //detour.target_func_info=std::make_unique<minhook::function_info_member>(&target_class::func);
    //target.func( );
    //detour.hook( );
    //detour.unhook_safe(boost::chrono::milliseconds(10));
    //target.func( );
    //DebugBreak( );

    boost::filesystem::path path = R"(C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\bin\)";
    boost::filesystem::path fs = "filesystem_stdio.dll";

    auto full_path = path / fs;

    utl::winapi::module_handle handle(LoadLibraryEx(full_path.c_str( ), nullptr,DONT_RESOLVE_DLL_REFERENCES));

    auto& md = utl::mem::all_modules::get( );
    auto& lib = *boost::range::find_if(md.all( ), [&](utl::mem::module_info& i) { return i.base( ) == handle.get(); });
    auto& vtables = lib.vtables( );
    vtables.load_from_memory( );
    auto& q = vtables.get_cache( );
    vtables.write_to_file("vtables.txt");
    /*curr.vtables( ).write_to_file();
    q.clear( );
    curr.vtables( ).load_from_file("vtables.txt");*/

    DebugBreak( );

    /*using namespace ranges;
int sum = accumulate(views::ints(1, unreachable) | views::transform([](int i) {
                         return i * i;
                     }) | views::take(10),
                     0);*/

    /*test_hooked          hook;
    minhook::hook_result s = hook.setup(test);
    test( );
    auto r = s.entry->set_state(true);
    test( );

    minhook::MH_remove_all_hooks( );
    test( );*/

    /*utl::hashed_vector<utl::hashed_string, int,utl::detail::_Hashed_vector_construct_helper_unique_fn> vec;
 auto aa=vec.find(utl::hashed_string_view("asd"));

 constexpr auto qweqe=std::_Weakly_equality_comparable_with<utl::hashed_string_view,utl::hashed_string>;

auto &val=vec.emplace("niggre",228);
 auto &test=vec.container_.back();
        auto aff=vec.find(test.first);*/

    //auto &test=vec.emplace(1,"stfu");

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness( );

    /*auto val = utl::dynamic_value<int>(5);
    val.set_new_value(4);
    auto q = val.changed( );
    val.set_new_value(4);
    auto q1 = val.changed( );
    val.new_value()=5;
    auto q2 = val.changed( );
     auto aa=std::move(val.new_value()=6);
    auto q3 = val.changed( );*/

    /*static constexpr auto test   = utl::crypt_str("nigger");
    auto                  result = test.unwrap_text<false>( );*/
    //auto ass=test.decrypt();

#if 0
    using ULT_CFG_STORAGE_NAMESPACE;

    auto root = utl::cfg_storage::root_category<nlohmann::json>( );

    root.values.add("test", 1);
    auto& c = root.childs.add("fuck").text( );
    struct anus
    {
        int                a = 228;
        std::array<int, 5> b = {1, 2, 3, 4, 5};
    };

    //auto qwe = std::as_const(root.values).find(1);

    anus an[5];

    auto aaa = utl::_Bytes_view(an);

    /* auto& a         = root.values.add("test2", anus( ));
     auto  rw_helper = root.create_rw_helper(std::filesystem::current_path( ), "test", "json");
     rw_helper.write( );
     a.get_value( ) = {1, 0, 0};
     rw_helper.load( );
 
     rw_helper.erase_file( );*/

    auto& modules = _UTL_MEMORY all_modules::get( );
    auto& m       = modules.current( );
    auto& vtables = m.vtables( );
    vtables.load( );

    auto test1 = m._File_on_disc(std::filesystem::current_path( ), "txt");

    vtables.write_to_file(std::filesystem::current_path( ) / "test.json");
    vtables.get_cache( ).clear( );
    vtables.load_from_file(std::filesystem::current_path( ) / "test.json");
#endif

    if (!helper.initialize( ))
        return 1;

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);

    ImFontConfig font_cfg;
    font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
    constexpr ImWchar ranges[] =
    {
        0x0020, 0xFFFF, //almost language of utf8 range
        0,
    };
    font_cfg.GlyphRanges = /*io.Fonts->GetGlyphRangesCyrillic( )*/ranges;
    auto font_cfg2 = font_cfg;

    auto& io = ImGui::GetIO( );

    io.FontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\ARIALUNI.TTF", 18.0f, &font_cfg, nullptr);

    //ImGuiFreeType::BuildFontAtlas(io.Fonts);
#if 0
    // Our state
    auto show_demo_window    = true;
    auto show_another_window = false;
    auto clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto d3d_device = helper.d3d_device( ).get( );


    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static auto f       = 0.0f;
            static auto counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                              // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", &reinterpret_cast<float&>(clear_color)); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine( );
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO( ).Framerate, ImGui::GetIO( ).Framerate);
            ImGui::End( );
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            {
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
            }
            ImGui::End( );
        }
#if 0
        constexpr auto flags = utl::make_bitflag(ImGuiWindowFlags_AlwaysAutoResize, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Begin("2048", nullptr, flags.get( ));
        {
            static auto playground = ImGui_ex::create_2048_playground(6, 6, font_arial, 0.15);
            playground( );
        }
        ImGui::End( );

        static auto test_file_browser = ImGui_ex::create_file_browser(R"(E:\)");
        test_file_browser( );
#endif

        // Rendering
        ImGui::EndFrame( );
        d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        const auto clear_col_dx = D3DCOLOR_RGBA(static_cast<int>(clear_color.x * 255.0f),
                                                static_cast<int>(clear_color.y * 255.0f),
                                                static_cast<int>(clear_color.z * 255.0f),
                                                static_cast<int>(clear_color.w * 255.0f));
        d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (d3d_device->BeginScene( ) >= 0)
        {
            ImGui::Render( );
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData( ));
            d3d_device->EndScene( );
        }
        const auto result = d3d_device->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && d3d_device->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET)
            helper.reset( );
    }


    ImGui_ImplDX9_Shutdown( );
    ImGui_ImplWin32_Shutdown( );
#endif

    helper.run( );

    return 0;
}
