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

	namespace csgo_interfaces
	{
		class csgo_interface_base
		{
		public:
			using sv = utl::string_view;
			using mb = utl::mem::memory_block;

			utl::mem::address addr( ) const;

			void from_interface(const sv& dll_name, const sv& interface_name);
			void from_sig(const mb& from, const sv& sig, size_t add, size_t deref);
			void from_sig(const sv& dll_name, const sv& sig, size_t add, size_t deref);
			void from_vfunc(void* instance, size_t index, size_t add, size_t deref);
			void from_ptr(void* ptr);

		private:
			void Set_result_assert_( ) const;

		protected:
			utl::mem::address result_;
		};

		template <class To, size_t Ptrs>
		class csgo_interface: public csgo_interface_base
		{
		public:
			using raw_pointer = decltype(detail::_Generate_pointer<To, Ptrs>( ));
			using pointer = To*;
			using reference = To&;

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

		public:
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
}

namespace cheat
{
	class csgo_interfaces final: public service_shared<csgo_interfaces, service_mode::async>
	{
		template <typename T, size_t Ptrs = 1>
		using ifc = detail::csgo_interfaces::csgo_interface<T, Ptrs>;

	public:
		~csgo_interfaces( ) override;
		csgo_interfaces( );

	protected:
		void Load( ) override;

	public:
		//utl::filesystem::path csgo_path;

		ifc<csgo::IBaseClientDLL>       client;
		ifc<csgo::IClientEntityList>    entity_list;
		ifc<csgo::IPrediction>          prediction;
		ifc<csgo::CGameMovement>        game_movement;
		ifc<csgo::IMDLCache>            mdl_cache;
		ifc<csgo::IVEngineClient>       engine;
		ifc<csgo::IVModelInfoClient>    mdl_info;
		ifc<csgo::IVModelRender>        mdl_render;
		ifc<csgo::IVRenderView>         render_view;
		ifc<csgo::IEngineTrace>         engine_trace;
		ifc<csgo::IVDebugOverlay>       debug_overlay;
		ifc<csgo::IGameEventManager2>   game_events;
		ifc<csgo::IEngineSound>         engine_sound;
		ifc<csgo::IMaterialSystem>      material_system;
		ifc<csgo::ICvar>                cvars;
		ifc<csgo::IPanel>               vgui_panel;
		ifc<csgo::ISurface>             vgui_surface;
		ifc<csgo::IPhysicsSurfaceProps> phys_props;
		ifc<csgo::IInputSystem>         input_sys;
		ifc<csgo::ClientModeShared>     client_mode;
		ifc<csgo::CGlobalVarsBase>      global_vars;
		ifc<csgo::CInput>               input;
		ifc<csgo::IMoveHelper>          move_helper;
		ifc<csgo::CGlowObjectManager>   glow_mgr;
		ifc<csgo::IViewRender>          view_render;
		ifc<csgo::CClientState>         client_state;
		ifc<csgo::IWeaponSystem>        weapon_sys;
		ifc<csgo::C_CSPlayer, 2>        local_player;
		ifc<IDirect3DDevice9>           d3d_device;
	};
}
