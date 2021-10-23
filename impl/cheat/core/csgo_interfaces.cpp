#include "csgo_interfaces.h"

#include "services_loader.h"
#include "console.h"

#ifndef CHEAT_GUI_TEST
#include "cheat/core/csgo_modules.h"
#include "cheat/csgo/IAppSystem.hpp"

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

	client        = (csgo_modules::client.find_game_interface<"VClient">( ));
	entity_list   = (csgo_modules::client.find_game_interface<"VClientEntityList">( ));
	prediction    = (csgo_modules::client.find_game_interface<"VClientPrediction">( ));
	game_movement = (csgo_modules::client.find_game_interface<"GameMovement">( ));

	engine        = (csgo_modules::engine.find_game_interface<"VEngineClient">( ));
	mdl_info      = (csgo_modules::engine.find_game_interface<"VModelInfoClient">( ));
	mdl_render    = (csgo_modules::engine.find_game_interface<"VEngineModel">( ));
	render_view   = (csgo_modules::engine.find_game_interface<"VEngineRenderView">( ));
	engine_trace  = (csgo_modules::engine.find_game_interface<"EngineTraceClient">( ));
	debug_overlay = (csgo_modules::engine.find_game_interface<"VDebugOverlay">( ));
	game_events   = (csgo_modules::engine.find_game_interface<"GAMEEVENTSMANAGER002">( ));
	engine_sound  = (csgo_modules::engine.find_game_interface<"IEngineSoundClient">( ));

	mdl_cache       = (csgo_modules::datacache.find_game_interface<"MDLCache">( ));
	material_system = (csgo_modules::materialsystem.find_game_interface<"VMaterialSystem">( ));
	cvars           = (csgo_modules::vstdlib.find_game_interface<"VEngineCvar">( ));
	vgui_panel      = (csgo_modules::vgui2.find_game_interface<"VGUI_Panel">( ));
	vgui_surface    = (csgo_modules::vguimatsurface.find_game_interface<"VGUI_Surface">( ));
	phys_props      = (csgo_modules::vphysics.find_game_interface<"VPhysicsSurfaceProps">( ));
	input_sys       = (csgo_modules::inputsystem.find_game_interface<"InputSystemVersion">( ));
	studio_renderer = (csgo_modules::studiorender.find_game_interface<"VStudioRender">( ));

	client_mode = get_vfunc(client, 10).add(5).deref(2);

	global_vars  = (csgo_modules::client.find_signature<"A1 ? ? ? ? 5E 8B 40 10">( ).add(1).deref(2));
	input        = (csgo_modules::client.find_signature<"B9 ? ? ? ? F3 0F 11 04 24 FF 50 10">( ).add(1).deref(1));
	move_helper  = (csgo_modules::client.find_signature<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">( ).add(2).deref(2));
	glow_mgr     = (csgo_modules::client.find_signature<"0F 11 05 ? ? ? ? 83 C8 01">( ).add(3).deref(1));
	view_render  = (csgo_modules::client.find_signature<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">( ).add(1).deref(1));
	weapon_sys   = (csgo_modules::client.find_signature<"8B 35 ? ? ? ? FF 10 0F B7 C0">( ).add(2).deref(1));
	local_player = (csgo_modules::client.find_signature<"8B 0D ? ? ? ? 83 FF FF 74 07">( ).add(2).deref(1));

	client_state = (csgo_modules::engine.find_signature<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">( ).add(1).deref(2));

	d3d_device = (csgo_modules::shaderapidx9.find_signature<"A1 ? ? ? ? 50 8B 08 FF 51 0C">( ).add(1).deref(2));
#endif

	co_return (true);
}

csgo_interfaces::csgo_interfaces( )
{
	this->wait_for_service<console>( );
}

CHEAT_REGISTER_SERVICE(csgo_interfaces);
