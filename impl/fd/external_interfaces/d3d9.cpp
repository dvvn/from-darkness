module;

#include <fd/core/object.h>
#include <fd/core/assert.h>

#include <d3d9.h>

module fd.d3d_device9;
import fd.application_info;
import fd.rt_modules;

FD_OBJECT_IMPL(IDirect3DDevice9, d3d_device9_raw, d3d_device9);

static auto _Find_d3d_device9(IDirect3DDevice9* known_ptr = nullptr)
{
    if (!known_ptr)
        known_ptr = fd::runtime_modules::shaderapidx9.find_interface_sig<"A1 ? ? ? ? 50 8B 08 FF 51 0C">().plus(1).deref<2>();

    return known_ptr;
}

d3d_device9_setter::d3d_device9_setter(IDirect3DDevice9* const ptr)
    : ptr_(_Find_d3d_device9(ptr))
{
}

IDirect3DDevice9* d3d_device9_setter::operator&() const
{
    return ptr_;
}

IDirect3DDevice9* d3d_device9_setter::operator->() const
{
    return ptr_;
}
