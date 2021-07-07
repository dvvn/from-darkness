#pragma once

//#include <windows.h>
#include <d3d9.h>

#include "win_user.h"

namespace utl::winapi
{
    template <std::derived_from<IUnknown> _Ty>
    class directx_deleter
    {
    public:
        using pointer = _Ty*;

        void operator()(_Ty* ptr) const
        {
            ptr->Release( );
        }
    };

    template <std::derived_from<IUnknown> _Ty>
    using d3d9 = std::unique_ptr<_Ty, directx_deleter<_Ty> >;

    class D3DPRESENT_PARAMETERS_SAFE
    {
    public:
        enum class present_intervals :UINT
        {
            INTERVAL_DEFAULT = D3DPRESENT_INTERVAL_DEFAULT,
            INTERVAL_ONE = D3DPRESENT_INTERVAL_ONE,
            INTERVAL_TWO = D3DPRESENT_INTERVAL_TWO,
            INTERVAL_THREE = D3DPRESENT_INTERVAL_THREE,
            INTERVAL_FOUR = D3DPRESENT_INTERVAL_FOUR,
            INTERVAL_IMMEDIATE = D3DPRESENT_INTERVAL_IMMEDIATE,
        };
        enum class present_flags :DWORD
        {
            LOCKABLE_BACKBUFFER = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER,
            DISCARD_DEPTHSTENCIL = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL,
            DEVICECLIP = D3DPRESENTFLAG_DEVICECLIP,
            VIDEO = D3DPRESENTFLAG_VIDEO,
        };

        using _Locked_fn = bool(__thiscall*)(void*);
        //using _Locked_fn_stored = std::_Front_binder<_Locked_fn, void*>;

        class _Locked_fn_data
        {
        public:
            ALLOW_COPY(_Locked_fn_data)

            _Locked_fn_data(_Locked_fn fn, void* arg): fn_(fn), arg_(arg) {}

            _Locked_fn_data(_Locked_fn_data&& other) noexcept
            {
                BOOST_ASSERT_MSG(arg_==nullptr, "Arg is set already!");

                this->fn_  = other.fn_;
                this->arg_ = other.arg_;
            }

            _Locked_fn_data& operator=(_Locked_fn_data&& other) noexcept
            {
                this->fn_ = other.fn_;
                return *this;
            }

            bool operator()( ) const { return fn_(arg_); }

        private:
            _Locked_fn fn_  = 0;
            void*      arg_ = 0;
        };
        using _Locked_fn_stored = _Locked_fn_data;

        template <typename _Ty>
        using _Setter_native = value_setter<_Ty,
                                            _Locked_fn_stored,
                                            value_setter_native_getter<_Ty>,
                                            value_setter_native_setter<_Ty> >;

        D3DPRESENT_PARAMETERS_SAFE(void* locked_fn_instance, _Locked_fn locked_fn) :
            locked_fn(locked_fn, locked_fn_instance), params_( ) { }

#if 1

        auto back_buffer_width( ) -> _Setter_native<UINT>;
        auto back_buffer_width( ) const -> UINT;
        auto back_buffer_height( ) -> _Setter_native<UINT>;
        auto back_buffer_height( ) const -> UINT;
#else
        struct back_buffer_size_data
        {
            bool operator==(const back_buffer_size_data& other) const = default;
            UINT width, height;
        };
        auto back_buffer_size()->_Setter_native<back_buffer_size_data>;
        auto back_buffer_size() const -> const back_buffer_size_data&;
#endif
        auto back_buffer_format( ) -> _Setter_native<D3DFORMAT>;
        auto back_buffer_format( ) const -> D3DFORMAT;
        auto back_buffer_count( ) -> _Setter_native<UINT>;
        auto back_buffer_count( ) const -> UINT;

        auto multi_sample_type( ) -> _Setter_native<D3DMULTISAMPLE_TYPE>;
        auto multi_sample_type( ) const -> D3DMULTISAMPLE_TYPE;
        auto multi_sample_quality( ) -> _Setter_native<DWORD>;
        auto multi_sample_quality( ) const -> DWORD;

        auto swap_effect( ) -> _Setter_native<D3DSWAPEFFECT>;
        auto swap_effect( ) const -> D3DSWAPEFFECT;
        auto owner_window( ) -> _Setter_native<HWND>;
        auto owner_window( ) const -> HWND;
        auto windowed( ) -> _Setter_native<BOOL>;
        auto windowed( ) const -> bool;
        auto enable_auto_depth_stencil( ) -> _Setter_native<BOOL>;
        auto enable_auto_depth_stencil( ) const -> bool;
        auto auto_depth_stencil_format( ) -> _Setter_native<D3DFORMAT>;
        auto auto_depth_stencil_format( ) const -> D3DFORMAT;
        auto flags( ) -> _Setter_native<bitflag<present_flags> >;
        auto flags( ) const -> present_flags;

        auto full_screen_refresh_rate_hz( ) -> _Setter_native<UINT>;
        auto full_screen_refresh_rate_hz( ) const -> UINT;
        auto presentation_interval( ) -> _Setter_native<present_intervals>;
        auto presentation_interval( ) const -> present_intervals;

        auto& raw( ) { return params_; }
        auto& raw( ) const { return params_; }

    private:
        template <typename _Ty>
        _Setter_native<_Ty> _Init_setter_native(_Ty& val) { return utl::make_value_setter(locked_fn, val); }

        _Locked_fn_data       locked_fn;
        D3DPRESENT_PARAMETERS params_;
    };

    class _D3D_interface_holder
    {
    public:
        enum class create_flags :DWORD
        {
            FPU_PRESERVE = D3DCREATE_FPU_PRESERVE,
            MULTITHREADED = D3DCREATE_MULTITHREADED,

            PUREDEVICE = D3DCREATE_PUREDEVICE,
            SOFTWARE_VERTEXPROCESSING = D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            HARDWARE_VERTEXPROCESSING = D3DCREATE_HARDWARE_VERTEXPROCESSING,
            MIXED_VERTEXPROCESSING = D3DCREATE_MIXED_VERTEXPROCESSING,

            DISABLE_DRIVER_MANAGEMENT = D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
            ADAPTERGROUP_DEVICE = D3DCREATE_ADAPTERGROUP_DEVICE,
        };

        void initialize( )
        {
            BOOST_ASSERT_MSG(!is_initialized(), "d3d9 interface already initialized!");
            const auto device = Direct3DCreate9(D3D_SDK_VERSION);
            BOOST_ASSERT_MSG(device != nullptr, "unable to initialize d3d9 interface!");
            holder_ = decltype(holder_)(device);
        }

        bool is_initialized( ) const
        {
            return holder_ != nullptr;
        }

        [[nodiscard]] IDirect3DDevice9* create_device(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, bitflag<create_flags> BehaviorFlags, D3DPRESENT_PARAMETERS& params) const;

    private:
        d3d9<IDirect3D9> holder_;
    };

    class _D3D_device_holder
    {
    public:
        //PREVENT_COPY_FORCE_MOVE(_D3D_device_holder);

        _D3D_device_holder( );

        auto& params( ) { return params_; }
        auto& params( ) const { return params_; }

        void set(IDirect3DDevice9* device)
        {
            BOOST_ASSERT_MSG(device!=nullptr, "target d3d device is null!");
            BOOST_ASSERT_MSG(!is_created(), "d3d device already created!");
            BOOST_ASSERT_MSG(!unlocked_, "d3d device is unlocked!");
            device_ = decltype(device_)(device);
        }

        IDirect3DDevice9*       get( ) { return device_.get( ); }
        const IDirect3DDevice9* get( ) const { return device_.get( ); }

        class _Unlocker: movable_class
        {
        public:
            PREVENT_COPY_FORCE_MOVE(_Unlocker);

            _Unlocker(_D3D_device_holder& owner) : owner_(owner)
            {
                owner_.get( ).unlock( );
            }

            ~_Unlocker( )
            {
                if (!this->moved( ))
                    owner_.get( ).lock( );
            }

            //void unlock( ) { owner_.get( ).unlocked_ = true; }
            //void lock( ) { owner_.get( ).unlocked_ = false; }

        private:
            std::reference_wrapper<_D3D_device_holder> owner_;
        };
        [[nodiscard]] _Unlocker unlock_lazy( ) { return *this; }

        void lock( ) { unlocked_ = false; }
        void unlock( ) { unlocked_ = true; }

        bool is_created( ) const;

        HRESULT reset( )
        {
            BOOST_ASSERT_MSG(is_created(), "d3d9 device not created!");
            return device_->Reset(&params_.raw( ));
        }

        const IDirect3DDevice9* operator->( ) const { return device_.get( ); }
        IDirect3DDevice9*       operator->( ) { return device_.get( ); }

    private:
        D3DPRESENT_PARAMETERS_SAFE params_;
        d3d9<IDirect3DDevice9>     device_;

        bool unlocked_ = false;
    };
}
