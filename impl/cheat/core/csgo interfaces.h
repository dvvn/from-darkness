#pragma once
#include "service.h"

namespace cheat::csgo
{
	class C_CSPlayer;
	class ClientModeShared;
	class C_BasePlayer;
	class IWeaponSystem;
	class CClientState;
	class IViewRender;
	class CGlowObjectManager;
	class IMoveHelper;
	class CInput;
	class CGlobalVarsBase;
	class IClientMode;
	class IInputSystem;
	class IPhysicsSurfaceProps;
	class ISurface;
	class IPanel;
	class ICvar;
	class IMaterialSystem;
	class IEngineSound;
	class IGameEventManager2;
	class IVDebugOverlay;
	class IEngineTrace;
	class IVRenderView;
	class IVModelRender;
	class IVModelInfoClient;
	class IVEngineClient;
	class IMDLCache;
	class CGameMovement;
	class IPrediction;
	class IClientEntityList;
	class IBaseClientDLL;
	class IStudioRender;
}

namespace cheat::detail
{
	template <typename T, size_t Num, size_t Counter = 0>
	constexpr auto _Generate_pointer( )
	{
		if constexpr (Num == Counter)
			return static_cast<T>(nullptr);
		else
			return _Generate_pointer<T*, Num, Counter + 1>( );
	}

	template <typename T>
	constexpr size_t _Count_pointers( )
	{
		if constexpr (!std::is_pointer_v<T>)
			return 0;
		else
			return _Count_pointers<std::remove_pointer_t<T>>( ) + 1;
	}

	class csgo_interface_base
	{
	protected:
		virtual ~csgo_interface_base( ) = default;

	public:
		using sv = utl::string_view;
		using mb = utl::memory_block;

		utl::address addr( ) const;

		void operator=(const utl::address& addr);

	private:
		void Set_result_assert_( ) const;

	protected:
		utl::address result_;
	};

	template <class To, size_t Ptrs>
	class csgo_interface final: public csgo_interface_base
	{
	public:
		using raw_pointer = decltype(detail::_Generate_pointer<To, Ptrs>( ));
		using pointer = To*;
		using reference = To&;
		using value_type = To;

		void operator=(const utl::address& addr)
		{
			*static_cast<csgo_interface_base*>(this) = addr;
		}

	private:
		pointer Pointer_( ) const
		{
			if constexpr (constexpr size_t deref = detail::_Count_pointers<raw_pointer>( ) - 1; deref > 0)
				return result_.deref(deref).cast<pointer>( );
			else
				return result_.cast<pointer>( );
		}

		reference Reference_( )
		{
			return *Pointer_( );
		}

		bool Is_null_( ) const
		{
			if (this->empty( ))
				return true;

			constexpr auto extra_deref = detail::_Count_pointers<raw_pointer>( ) - 1;
			if constexpr (extra_deref > 0)
			{
				auto addr = result_.value( );
				for (size_t deref = 0; deref < extra_deref; ++deref)
				{
					addr = *reinterpret_cast<uintptr_t*>(addr);
					if (addr == 0u)
						return true;
				}
			}

			return false;
		}

	public:
		bool operator==(nullptr_t) const
		{
			return Is_null_( );
		}

		bool operator!=(nullptr_t) const
		{
			return !((*this) == nullptr);
		}

		template <std::derived_from<To> T=To>
		bool operator==(const T* other) const
		{
			return Pointer_( ) != other;
		}

		template <std::derived_from<To> T=To>
		bool operator!=(const T* other) const
		{
			return !((*this) == other);
		}

		operator To*( ) const
		{
			return Pointer_( );
		}

		To* operator->( ) const
		{
			return Pointer_( );
		}

		To& operator*( )
		{
			return Reference_( );
		}

		To* get( ) const
		{
			return Pointer_( );
		}

		bool empty( ) const
		{
			return result_ == 0u;
		}
	};
}

namespace cheat
{
	class csgo_interfaces final: public service<csgo_interfaces>
	{
		template <typename T, size_t Ptrs = 1>
		using ifc = detail::csgo_interface<T, Ptrs>;

	protected:
		bool Do_load( ) override;

	public:
		//utl::filesystem::path csgo_path;

		ifc<csgo::IBaseClientDLL> client;
		ifc<csgo::IClientEntityList> entity_list;
		ifc<csgo::IPrediction> prediction;
		ifc<csgo::CGameMovement> game_movement;
		ifc<csgo::IMDLCache> mdl_cache;
		ifc<csgo::IVEngineClient> engine;
		ifc<csgo::IVModelInfoClient> mdl_info;
		ifc<csgo::IVModelRender> mdl_render;
		ifc<csgo::IVRenderView> render_view;
		ifc<csgo::IEngineTrace> engine_trace;
		ifc<csgo::IVDebugOverlay> debug_overlay;
		ifc<csgo::IGameEventManager2> game_events;
		ifc<csgo::IEngineSound> engine_sound;
		ifc<csgo::IMaterialSystem> material_system;
		ifc<csgo::ICvar> cvars;
		ifc<csgo::IPanel> vgui_panel;
		ifc<csgo::ISurface> vgui_surface;
		ifc<csgo::IPhysicsSurfaceProps> phys_props;
		ifc<csgo::IInputSystem> input_sys;
		ifc<csgo::ClientModeShared> client_mode;
		ifc<csgo::CGlobalVarsBase> global_vars;
		ifc<csgo::CInput> input;
		ifc<csgo::IMoveHelper> move_helper;
		ifc<csgo::CGlowObjectManager> glow_mgr;
		ifc<csgo::IViewRender> view_render;
		ifc<csgo::CClientState> client_state;
		ifc<csgo::IWeaponSystem> weapon_sys;
		ifc<csgo::IStudioRender> studio_renderer;
		ifc<csgo::C_CSPlayer, 2> local_player;
		ifc<IDirect3DDevice9> d3d_device;
	};
}
