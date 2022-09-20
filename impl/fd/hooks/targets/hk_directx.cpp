module;

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <optional>

module fd.hooks.directx;
import fd.functional.invoke;
import fd.library_info;

using namespace fd;
using namespace hooks;

struct render_interface
{
    ~render_interface()
    {
        if (library_info::_Find(L"d3d9.dll")) // todo: find better way
            ImGui_ImplDX9_Shutdown();
    }

    render_interface(IDirect3DDevice9* inst)
    {
        if (!ImGui_ImplDX9_Init(inst))
            std::terminate();
    }

    void release_textures()
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    bool render(IDirect3DDevice9* inst)
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        // todo: return if minimized

        ImGui::NewFrame();
        {
            // invoke(gui::menu);
            ImGui::ShowDemoWindow();
        }
        ImGui::EndFrame();

        const auto bg = inst->BeginScene();
        IM_ASSERT(bg == D3D_OK);
#ifdef NDEBUG
        if (bg != D3D_OK)
            return false;
#endif
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
        const auto ed = inst->EndScene();
        IM_ASSERT(ed == D3D_OK);
#ifdef NDEBUG
        if (ed != D3D_OK)
            return false;
#endif

        return true;
    }
};

static std::optional<render_interface> _Render_interface;

static d3d9_reset* _D3d9_reset;

d3d9_reset::~d3d9_reset()
{
    // added for logging only
    if (*this)
        this->disable();
}

d3d9_reset::d3d9_reset(IDirect3DDevice9* inst)
{
    this->init({ inst, 16 }, &d3d9_reset::callback);
    if (!_Render_interface)
        _Render_interface = inst;
    _D3d9_reset = this;
}

d3d9_reset::d3d9_reset(d3d9_reset&& other)
    : impl(std::move(other))
{
    _D3d9_reset = this;
}

string_view d3d9_reset::name() const
{
    return "IDirect3DDevice9::Reset";
}

bool d3d9_reset::disable()
{
    const auto ok = impl::disable();
    if (ok)
        _Render_interface.reset();
    return ok;
}

void WINAPI d3d9_reset::callback(D3DPRESENT_PARAMETERS* params)
{
    _Render_interface->release_textures();
    invoke(&d3d9_reset::callback, _D3d9_reset->get_original_method(), this, params);
}

//------------

static d3d9_present* _D3d9_present;

d3d9_present::~d3d9_present()
{
    // added for logging only
    if (*this)
        this->disable();
}

d3d9_present::d3d9_present(IDirect3DDevice9* inst)
{
    this->init({ inst, 17 }, &d3d9_present::callback);
    if (!_Render_interface)
        _Render_interface = inst;
    _D3d9_present = this;
}

d3d9_present::d3d9_present(d3d9_present&& other)
    : impl(std::move(other))
{
    _D3d9_present = this;
}

string_view d3d9_present::name() const
{
    return "IDirect3DDevice9::Present";
}

bool d3d9_present::disable()
{
    const auto ok = impl::disable();
    if (ok)
        _Render_interface.reset();
    return ok;
}

HRESULT WINAPI d3d9_present::callback(THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    if (!_Render_interface->render(reinterpret_cast<IDirect3DDevice9*>(this)))
        std::terminate();
    return invoke(&d3d9_present::callback, _D3d9_present->get_original_method(), this, source_rect, desc_rect, dest_window_override, dirty_region);
}
