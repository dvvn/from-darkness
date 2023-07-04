#include "dx9.h"
//
#include "pattern.h"
#include "diagnostics/runtime_error.h"
#include "library_info/system.h"
#ifdef _DEBUG
#include "string/view.h"
#endif

#include <d3d9.h>

#include <cassert>

namespace fd
{
dx9_backend_native::~dx9_backend_native()
{
    dx9_backend_native::destroy();
}

dx9_backend_native::dx9_backend_native(pointer device)
    : basic_dx9_backend(*device)
    , device_(device)
{
}

dx9_backend_native::dx9_backend_native(system_library_info info)
    : dx9_backend_native([info] {
        assert(info.name() == L"shaderapidx9.dll");
        auto addr = info.pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C"_pat);
        if (!addr)
            throw runtime_error("Unable to find DX9 device!");
        return *reinterpret_cast<pointer *>(static_cast<uint8_t *>(addr) + 1);
    }())
{
}

void dx9_backend_native::destroy()
{
    if ((*device_))
        basic_dx9_backend::destroy();
}

void dx9_backend_native::render(ImDrawData *draw_data)
{
    (void)(*device_)->BeginScene();
    basic_dx9_backend::render(draw_data);
    (void)(*device_)->EndScene();
}

}