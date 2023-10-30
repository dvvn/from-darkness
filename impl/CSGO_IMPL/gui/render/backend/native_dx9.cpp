#include "native_dx9.h"
//
#include "noncopyable.h"
#include "diagnostics/runtime_error.h"
#include "library_info/system.h"
#include "memory/pattern.h"
#ifdef _DEBUG
#include "string/view.h"
#endif

#include <d3d9.h>
#include <imgui.h>

#include <cassert>

namespace fd
{
struct native_dx9_backend_protector
{
    ~native_dx9_backend_protector();
};

class native_dx9_backend final : basic_dx9_backend, public native_dx9_backend_protector, public noncopyable
{
    friend struct native_dx9_backend_protector;

    using pointer = IDirect3DDevice9**;

    pointer device_;

  public:
    native_dx9_backend(pointer device)
        : basic_dx9_backend(*device)
        , device_(device)
    {
    }

    native_dx9_backend(system_library_info info)
        : native_dx9_backend([info] {
            assert(info.name() == L"shaderapidx9.dll");
            auto const addr = info.pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C"_pat);
            if (!addr)
                throw runtime_error("Unable to find DX9 device!");
            return *reinterpret_cast<pointer*>(static_cast<uint8_t*>(addr) + 1);
        }())
    {
    }

    using basic_dx9_backend::new_frame;

    void render(ImDrawData* draw_data)
    {
        (void)(*device_)->BeginScene();
        basic_dx9_backend::render(draw_data);
        (void)(*device_)->EndScene();
    }

    /*template <typename Ret, typename... Args>
    auto operator[](Ret (__stdcall IDirect3DDevice9::*func)(Args...)) const
        -> vfunc<call_type::stdcall_, Ret, IDirect3DDevice9, Args...>
    {
        return {func, *device_};
    }*/

    IDirect3DDevice9* native() const
    {
        return *device_;
    }
};

namespace detail
{
struct simple_imgui_dx9_data
{
    IDirect3DDevice9* device;
};
} // namespace detail

native_dx9_backend_protector::~native_dx9_backend_protector()
{
    if (*static_cast<native_dx9_backend*>(this)->device_ == nullptr) // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        static_cast<detail::simple_imgui_dx9_data*>(ImGui::GetIO().BackendRendererUserData)->device = nullptr;
}

} // namespace fd