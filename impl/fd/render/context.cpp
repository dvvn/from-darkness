#include "context.h"

#include "fd/library_info/system.h"
#include "fd/tool/algorithm/pattern.h"
#ifdef _DEBUG
#include "fd/tool/string.h"
#endif

namespace fd
{
template <typename T>
static auto get_creation_params(T *ptr)
{
    D3DDEVICE_CREATION_PARAMETERS creation_parameters;
    if (FAILED(ptr->GetCreationParameters(&creation_parameters)))
        std::unreachable();
    return creation_parameters;
}

namespace detail
{
internal_render_backend<IDirect3DDevice9>::internal_render_backend(system_library_info info)
    : device_(*reinterpret_cast<IDirect3DDevice9 ***>(
          static_cast<uint8_t *>(info.pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C"_pat)) + 1))
{
    assert(info.name() == _T("shaderapidx9.dll"));
}

IDirect3DDevice9 *internal_render_backend<IDirect3DDevice9>::backend() const
{
    return *device_;
}

HWND internal_render_backend<IDirect3DDevice9>::window() const
{
    return get_creation_params(*device_).hFocusWindow;
}

WNDPROC internal_render_backend<IDirect3DDevice9>::window_proc() const
{
    auto w = get_creation_params(*device_).hFocusWindow;
    return reinterpret_cast<WNDPROC>(
        std::invoke(IsWindowUnicode(w) ? GetWindowLongPtrW : GetWindowLongPtrA, w, GWLP_WNDPROC));
}

WNDPROC internal_render_backend<IDirect3DDevice9>::default_window_proc() const
{
    auto w = get_creation_params(*device_).hFocusWindow;
    return IsWindowUnicode(w) ? DefWindowProcW : DefWindowProcA;
}
} // namespace detail

render_context<true>::render_context()
    : own_render_backend(_T("Unnamed"))
    , basic_render_context(window(), backend())
{
}

render_context<false>::~render_context()
{
    if (!backend())
        detach();
}

render_context<false>::render_context(system_library_info info)
    : detail::internal_render_backend<FD_RENDER_BACKEND>(info)
    , basic_render_context(window(), backend())
{
}

}