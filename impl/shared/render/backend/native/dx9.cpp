#include "dx9.h"
#include "noncopyable.h"
#include "diagnostics/runtime_error.h"
#include "library_info/system.h"
#include "memory/pattern.h"
#ifdef _DEBUG
#include "string/view.h"
#endif

#include <d3d9.h>

#include <cassert>

namespace fd
{
class native_dx9_backend final : public basic_dx9_backend
{
    using pointer = IDirect3DDevice9**;

    pointer device_;

    native_dx9_backend(pointer device)
        : basic_dx9_backend(*device)
        , device_(device)
    {
    }

  public:
    ~native_dx9_backend() override
    {
        native_dx9_backend::destroy();
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

    void destroy() override
    {
        if (*device_)
            basic_dx9_backend::destroy();
    }

    void render(ImDrawData* draw_data) override
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

    void* native() const override
    {
        return *device_;
    }
};

FD_OBJECT_IMPL(native_dx9_backend);
} // namespace fd