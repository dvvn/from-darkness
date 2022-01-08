module;

#include "cheat/service/includes.h"
#include <nstd/mem/address_includes.h>
#include <nstd/type_traits.h>
#include <d3d9.h>

export module cheat.csgo.interfaces;

export import :ModelRender;
export import :BaseClient;
export import :ClientEntityList;
export import :Prediction;
export import :MDLCache;
export import :EngineClient;
export import :ModelInfoClient;
export import :RenderView;
export import :EngineTrace;
export import :DebugOverlay;
export import :GameEvents;
export import :EngineSound;
export import :MaterialSystem;
export import :ConVar;
export import :VguiPanel;
export import :VguiSurface;
export import :Physics;
export import :InputSystem;
export import :ClientMode;
export import :GlobalVars;
export import :Input;
export import :MoveHelper;
export import :GlowEffect;
export import :ViewRender;
export import :ClientState;
export import :WeaponSystem;
export import :StudioRender;

export import :C_BaseCombatWeapon;
export import :C_CSPlayer;
export import :Recv;
export import :DataMap;
export import :UserCmd;

export import cheat.service;
export import nstd.mem;

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

/*template <typename T>
constexpr auto remove_all_pointers( )
{
	if constexpr (std::is_pointer_v<T>)
		return remove_all_pointers<std::remove_pointer_t<T>>( );
	else
		return std::type_identity<T>( );
}*/
export namespace cheat
{
	template <class To, size_t Ptrs>
	class csgo_interface final
	{
	public:
		using raw_pointer = decltype(_Generate_pointer<To, Ptrs>( ));
		using pointer = To*;
		using reference = To&;
		using value_type = To;

		csgo_interface( ) = default;

		csgo_interface(raw_pointer ptr)
			: result_(ptr)
		{
		}

		bool empty( ) const
		{
			return result_ == nullptr;
		}

		nstd::mem::address addr( ) const
		{
			return result_;
		}

		nstd::mem::address vfunc(ptrdiff_t index) const
		{
			return (result_.ref<nstd::mem::address*>( )[index]);
		}

		void operator=(const nstd::mem::address& addr)
		{
#ifdef _DEBUG
			if (!empty( ))
				throw;
#endif

			result_ = addr;
		}

	private:
		pointer get_pointer( ) const
		{
			if constexpr (constexpr size_t deref = _Count_pointers<raw_pointer>( ) - 1; deref > 0)
				return result_.deref(deref).cast<pointer>( );
			else
				return result_.cast<pointer>( );
		}

		reference get_reference( )
		{
			return *get_pointer( );
		}

		bool is_null( ) const
		{
			if (this->empty( ))
				return true;

			constexpr auto extra_deref = _Count_pointers<raw_pointer>( ) - 1;
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
		/*bool operator==(nullptr_t) const { return is_null( ); }
		bool operator!=(nullptr_t) const { return !(*this == nullptr); }*/

		/*template <std::derived_from<To> T = To>
		bool operator==(const T* other) const { return get_pointer( ) != other; }

		template <std::derived_from<To> T = To>
		bool operator!=(const T* other) const { return !(*this == other); }*/

		operator pointer( ) const { return get_pointer( ); }

		pointer operator->( ) const { return get_pointer( ); }
		reference operator*( ) { return get_reference( ); }
		pointer get( ) const { return get_pointer( ); }

	private:
		nstd::mem::address result_;
	};

	template <typename T>
	csgo_interface(T)->csgo_interface<nstd::remove_all_pointers_t<T>, _Count_pointers<T>( )>;

	class csgo_interfaces final : public dynamic_service<csgo_interfaces>
	{
		template <typename T, size_t Ptrs = 1>
		using ifc = csgo_interface<T, Ptrs>;

	protected:
		bool load_impl( ) noexcept override;
		void load_async( ) noexcept override;

	public:
		//nstd::filesystem::path csgo_path;

		csgo_interfaces( );

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
		ifc<csgo::IConVar> cvars;
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
