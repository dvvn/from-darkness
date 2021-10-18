#include "csgo interfaces.h"

#include "services loader.h"
#include "console.h"

#ifndef CHEAT_GUI_TEST
#include "cheat/core/csgo modules.h"
#include "cheat/sdk/IAppSystem.hpp"

#include <dhooks/hook_utils.h>
#else
#include <nstd/runtime_assert_fwd.h>
#endif

//#include <d3d9.h>

using namespace cheat;
using namespace detail;
using namespace csgo;

#ifndef CHEAT_GUI_TEST
// ReSharper disable once CppInconsistentNaming
 static nstd::address get_vfunc(void* instance, size_t index)
{
	return dhooks::_Pointer_to_virtual_class_table(instance)[index];
}
#endif

nstd::address csgo_interface_base::addr( ) const
{
	return result_;
}

csgo_interface_base& csgo_interface_base::operator=(const nstd::address& addr)
{
	Set_result_assert_( );
	result_ = addr;
	return *this;
}

void csgo_interface_base::Set_result_assert_( ) const
{
	runtime_assert(result_ == 0u, "Result already set!");
}

#ifdef CHEAT_GUI_TEST
typedef struct IDirect3DDevice9 *LPDIRECT3DDEVICE9, *PDIRECT3DDEVICE9;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
#endif

service_base::load_result csgo_interfaces::load_impl( ) noexcept
{
	//unused
#if 0
#ifndef CHEAT_GUI_TEST
	csgo_path = all_modules::get_ptr( )->owner( ).work_dir( );
#else
	using string_type = filesystem::path::string_type;
	const auto steam_path = filesystem::path(winreg::RegKey(HKEY_CURRENT_USER, L"Software\\Valve\\Steam").GetStringValue(L"SteamPath")).make_preferred( );

	for(auto& dir : filesystem::directory_iterator(steam_path / L"steamapps" / L"common"))
	{
		const auto file = dir / L"steam_appid.txt";
		if(!exists(file))
			continue;

		std::ifstream ifs(file.native( ));
		using it = std::istreambuf_iterator<decltype(ifs)::char_type>;

		const auto str = std::string(it(ifs), it( ));

		if(str.starts_with("730") && (str.size( ) == 3 || !std::isdigit(str[3])))
		{
			csgo_path = dir;
			break;
		}
	}
	runtime_assert(!csgo_path.empty( ), "Csgo path not found!");

#if 0
	const auto csgo_bin = csgo_path / L"csgo" / L"bin";
	const auto bin = csgo_path / L"bin";

	//unable to load interfaces manually

	const auto load_lib = [](const filesystem::path& from, const wstring_view& name)
	{
		const auto full_path = from / name += L".dll";
		const auto handle = LoadLibraryExW(full_path.c_str( ), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH | DONT_RESOLVE_DLL_REFERENCES);
		(void)handle;
		//no memory leak here. it unloads on app close
		runtime_assert(handle != 0, "Unable to load library!");
	};

	load_lib(csgo_bin, L"client");

	load_lib(bin, L"engine");
	load_lib(bin, L"datacache");
	load_lib(bin, L"vstdlib");
	load_lib(bin, L"vgui2");
	load_lib(bin, L"vguimatsurface");
	load_lib(bin, L"vphysics");
	load_lib(bin, L"inputsystem");

	all_modules::get_ptr( )->update(true);

#endif

#endif
#endif

#ifdef CHEAT_GUI_TEST

	d3d_device = (g_pd3dDevice);
#else
	using namespace nstd::address_pipe;

	client        = CHEAT_FIND_GAME_INTERFACE(client, "VClient");
	entity_list   = CHEAT_FIND_GAME_INTERFACE(client, "VClientEntityList");
	prediction    = CHEAT_FIND_GAME_INTERFACE(client, "VClientPrediction");
	game_movement = CHEAT_FIND_GAME_INTERFACE(client, "GameMovement");

	engine        = CHEAT_FIND_GAME_INTERFACE(engine, "VEngineClient");
	mdl_info      = CHEAT_FIND_GAME_INTERFACE(engine, "VModelInfoClient");
	mdl_render    = CHEAT_FIND_GAME_INTERFACE(engine, "VEngineModel");
	render_view   = CHEAT_FIND_GAME_INTERFACE(engine, "VEngineRenderView");
	engine_trace  = CHEAT_FIND_GAME_INTERFACE(engine, "EngineTraceClient");
	debug_overlay = CHEAT_FIND_GAME_INTERFACE(engine, "VDebugOverlay");
	game_events   = CHEAT_FIND_GAME_INTERFACE(engine, "GAMEEVENTSMANAGER002");
	engine_sound  = CHEAT_FIND_GAME_INTERFACE(engine, "IEngineSoundClient");

	mdl_cache       = CHEAT_FIND_GAME_INTERFACE(datacache, "MDLCache");
	material_system = CHEAT_FIND_GAME_INTERFACE(materialsystem, "VMaterialSystem");
	cvars           = CHEAT_FIND_GAME_INTERFACE(vstdlib, "VEngineCvar");
	vgui_panel      = CHEAT_FIND_GAME_INTERFACE(vgui2, "VGUI_Panel");
	vgui_surface    = CHEAT_FIND_GAME_INTERFACE(vguimatsurface, "VGUI_Surface");
	phys_props      = CHEAT_FIND_GAME_INTERFACE(vphysics, "VPhysicsSurfaceProps");
	input_sys       = CHEAT_FIND_GAME_INTERFACE(inputsystem, "InputSystemVersion");
	studio_renderer = CHEAT_FIND_GAME_INTERFACE(studiorender, "VStudioRender");

	client_mode = get_vfunc(client, 10).add(5).deref(2);

	global_vars  = CHEAT_FIND_SIG(client, "A1 ? ? ? ? 5E 8B 40 10", add(1), deref(2));
	input        = CHEAT_FIND_SIG(client, "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10", add(1), deref(1));
	move_helper  = CHEAT_FIND_SIG(client, "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01", add(2), deref(2));
	glow_mgr     = CHEAT_FIND_SIG(client, "0F 11 05 ? ? ? ? 83 C8 01", add(3), deref(1));
	view_render  = CHEAT_FIND_SIG(client, "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10", add(1), deref(1));
	weapon_sys   = CHEAT_FIND_SIG(client, "8B 35 ? ? ? ? FF 10 0F B7 C0", add(2), deref(1));
	local_player = CHEAT_FIND_SIG(client, "8B 0D ? ? ? ? 83 FF FF 74 07", add(2), deref(1));

	client_state = CHEAT_FIND_SIG(engine, "A1 ? ? ? ? 8B 80 ? ? ? ? C3", add(1), deref(2));

	d3d_device = CHEAT_FIND_SIG(shaderapidx9, "A1 ? ? ? ? 50 8B 08 FF 51 0C", add(1), deref(2));
#endif

	co_return (true);
}

csgo_interfaces::csgo_interfaces( )
{
	this->wait_for_service<console>( );
}

CHEAT_REGISTER_SERVICE(csgo_interfaces);
