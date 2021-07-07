#include "minhook_cpp/hook_utils.h"
#if __has_include("d3d9.h")

#include "d3d9_ex.h"
#pragma comment(lib, "d3d9.lib")
using namespace utl::winapi;

#if 1
auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_width( ) -> _Setter_native<UINT>
{
    return _Init_setter_native(params_.BackBufferWidth);
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_width( ) const -> UINT
{
    return params_.BackBufferWidth;
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_height( ) -> _Setter_native<UINT>
{
    return _Init_setter_native(params_.BackBufferHeight);
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_height( ) const -> UINT
{
    return params_.BackBufferHeight;
}
#else
auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_size( ) -> __Setter_native<back_buffer_size_data>
{
    return _Init_setter_native(reinterpret_cast<back_buffer_size_data&>(params_.BackBufferWidth));
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_size( ) const -> const back_buffer_size_data&
{
    return reinterpret_cast<const back_buffer_size_data&>(params_.BackBufferWidth);
}
#endif
auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_format( ) -> _Setter_native<D3DFORMAT>
{
    return _Init_setter_native(params_.BackBufferFormat);
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_format( ) const -> D3DFORMAT
{
    return params_.BackBufferFormat;
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_count( ) -> _Setter_native<UINT>
{
    return _Init_setter_native(params_.BackBufferCount);
}

auto D3DPRESENT_PARAMETERS_SAFE::back_buffer_count( ) const -> UINT
{
    return params_.BackBufferCount;
}

auto D3DPRESENT_PARAMETERS_SAFE::multi_sample_type( ) -> _Setter_native<D3DMULTISAMPLE_TYPE>
{
    return _Init_setter_native(params_.MultiSampleType);
}

auto D3DPRESENT_PARAMETERS_SAFE::multi_sample_type( ) const -> D3DMULTISAMPLE_TYPE
{
    return params_.MultiSampleType;
}

auto D3DPRESENT_PARAMETERS_SAFE::multi_sample_quality( ) -> _Setter_native<DWORD>
{
    return _Init_setter_native(params_.MultiSampleQuality);
}

auto D3DPRESENT_PARAMETERS_SAFE::multi_sample_quality( ) const -> DWORD
{
    return params_.MultiSampleQuality;
}

auto D3DPRESENT_PARAMETERS_SAFE::swap_effect( ) -> _Setter_native<D3DSWAPEFFECT>
{
    return _Init_setter_native(params_.SwapEffect);
}

auto D3DPRESENT_PARAMETERS_SAFE::swap_effect( ) const -> D3DSWAPEFFECT
{
    return params_.SwapEffect;
}

auto D3DPRESENT_PARAMETERS_SAFE::owner_window( ) -> _Setter_native<HWND>
{
    return _Init_setter_native(params_.hDeviceWindow);
}

auto D3DPRESENT_PARAMETERS_SAFE::owner_window( ) const -> HWND
{
    return params_.hDeviceWindow;
}

auto D3DPRESENT_PARAMETERS_SAFE::windowed( ) -> _Setter_native<BOOL>
{
    return _Init_setter_native(params_.Windowed);
}

auto D3DPRESENT_PARAMETERS_SAFE::windowed( ) const -> bool
{
    return params_.Windowed;
}

auto D3DPRESENT_PARAMETERS_SAFE::enable_auto_depth_stencil( ) -> _Setter_native<BOOL>
{
    return _Init_setter_native(params_.EnableAutoDepthStencil);
}

auto D3DPRESENT_PARAMETERS_SAFE::enable_auto_depth_stencil( ) const -> bool
{
    return params_.EnableAutoDepthStencil;
}

auto D3DPRESENT_PARAMETERS_SAFE::auto_depth_stencil_format( ) -> _Setter_native<D3DFORMAT>
{
    return _Init_setter_native(params_.AutoDepthStencilFormat);
}

auto D3DPRESENT_PARAMETERS_SAFE::auto_depth_stencil_format( ) const -> D3DFORMAT
{
    return params_.AutoDepthStencilFormat;
}

auto D3DPRESENT_PARAMETERS_SAFE::flags( ) -> _Setter_native<bitflag<present_flags> >
{
    return _Init_setter_native(reinterpret_cast<bitflag<present_flags>&>(params_.Flags));
}

auto D3DPRESENT_PARAMETERS_SAFE::flags( ) const -> present_flags
{
    return static_cast<present_flags>(params_.Flags);
}

auto D3DPRESENT_PARAMETERS_SAFE::full_screen_refresh_rate_hz( ) -> _Setter_native<UINT>
{
    return _Init_setter_native(params_.FullScreen_RefreshRateInHz);
}

auto D3DPRESENT_PARAMETERS_SAFE::full_screen_refresh_rate_hz( ) const -> UINT
{
    return params_.FullScreen_RefreshRateInHz;
}

auto D3DPRESENT_PARAMETERS_SAFE::presentation_interval( ) -> _Setter_native<present_intervals>
{
    return _Init_setter_native(reinterpret_cast<present_intervals&>(params_.PresentationInterval));
}

auto D3DPRESENT_PARAMETERS_SAFE::presentation_interval( ) const -> present_intervals
{
    return static_cast<present_intervals>(params_.PresentationInterval);
}

IDirect3DDevice9* _D3D_interface_holder::create_device(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, bitflag<create_flags> BehaviorFlags, D3DPRESENT_PARAMETERS& params) const
{
    BOOST_ASSERT_MSG(is_initialized(), "d3d9 device not initialized!");
    IDirect3DDevice9* d3d_device    = nullptr;
    const auto        create_result = this->holder_->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags.convert( ), &params, &d3d_device);
    if (FAILED(create_result))
    {
        BOOST_ASSERT("Unable to create d3d9 device");
        return nullptr;
    }
    else
    {
        //device_ = decltype(device_)(d3d_device);
        return d3d_device;
    }
}

_D3D_device_holder::_D3D_device_holder( ):
    params_(this, static_cast<D3DPRESENT_PARAMETERS_SAFE::_Locked_fn>(minhook::pointer_to_class_method(&_D3D_device_holder::is_created))) {}

bool _D3D_device_holder::is_created( ) const
{
    if (this->device_ == nullptr)
        return false;
    if (this->unlocked_)
        return false;

    return true;
}

#endif
