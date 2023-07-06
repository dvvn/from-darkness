#pragma once
#include "noncopyable.h"
#include "functional/vfunc.h"
#include "render/backend/basic_dx9.h"

namespace fd
{
class system_library_info;

struct dx9_backend_native final : basic_dx9_backend, noncopyable
{
    using pointer = IDirect3DDevice9 **;

  private:
    pointer device_;

    dx9_backend_native(pointer device);

  public:
    ~dx9_backend_native();
    dx9_backend_native(system_library_info info);

    void destroy() override;
    void render(ImDrawData *draw_data) override;

    template <typename Ret, typename... Args>
    auto operator[](Ret (__stdcall IDirect3DDevice9::*func)(Args...)) const
        -> vfunc<call_type_t::stdcall_, Ret, IDirect3DDevice9, Args...>
    {
        return {func, *device_};
    }
};
} // namespace fd