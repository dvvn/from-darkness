#include "csgo interfaces.h"

#include "console.h"
#include "helpers.h"

#include "cheat/sdk/IAppSystem.hpp"

#include "cheat/utils/signature.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

class CInterfaceRegister
{
public:
	InstantiateInterfaceFn create_fn;
	const char*            name;
	CInterfaceRegister*    next;
};

class interfaces_cache
{
public:
	using entry_type = nstd::unordered_map<std::string_view, InstantiateInterfaceFn>;
	using cache_type = nstd::ordered_map<module_info*, entry_type>;

private:
	cache_type cache__;

	const entry_type& Get_entry_(const std::string_view& dll_name)
	{
		constexpr auto fill_entry = [](entry_type& entry, module_info* info)
		{
			runtime_assert(entry.empty(), "Entry already filled!");

			auto& exports = info->exports( );

			[[maybe_unused]] const auto load_result = exports.load( );
			runtime_assert(load_result ==true, "Unable to load exports");

			const auto& create_fn = exports.get_cache( ).at("CreateInterface");
			const auto  reg       = create_fn.rel32(0x5).add(0x6).deref(2).ptr<CInterfaceRegister>( );

			auto temp_entry = std::vector<entry_type::value_type>( );
			for (auto r = reg; r != nullptr; r = r->next)
				temp_entry.emplace_back(make_pair(std::string_view(r->name), r->create_fn));

			const auto contains_duplicate = [&](const std::string_view& new_string, size_t original_size)
			{
				auto detected = false;
				for (auto& raw_string: temp_entry | ranges::views::keys)
				{
					if (raw_string.size( ) != original_size)
						continue;
					if (!raw_string.starts_with(new_string))
						continue;
					if (detected)
						return true;
					detected = true;
				}
				return false;
			};
			const auto drop_underline = [&](const std::string_view& str, size_t original_size) -> std::optional<std::string_view>
			{
				if (str.ends_with('_'))
				{
					if (const auto str2 = std::string_view(str.begin( ), str.end( ) - 1); !contains_duplicate(str2, original_size))
						return str2;
				}
				return { };
			};
			const auto get_pretty_string = [&](const std::string_view& str) -> std::optional<std::string_view>
			{
				size_t remove = 0;
				for (const auto c: str | ranges::views::reverse)
				{
					if (!std::isdigit(c))
						break;

					++remove;
				}

				const auto original_size = str.size( );

				if (remove == 0)
					return drop_underline(str, original_size);

				auto str2 = str;
				str2.remove_suffix(remove);
				if (contains_duplicate(str2, original_size))
					return drop_underline(str, original_size);
				return drop_underline(str2, original_size).value_or(str2);
			};

			for (const auto& [name, fn]: temp_entry)
			{
				const auto name_pretty = get_pretty_string(name);
				entry.emplace(name_pretty.value_or(name), fn);
			}
		};

		for (const auto& [module, entry]: cache__)
		{
			if (module->name( ) == dll_name)
				return entry;
		}

		const auto info = all_modules::get_ptr( )->find(dll_name);

		auto& entry = cache__[info];
		runtime_assert(entry.empty( ));
		fill_entry(entry, info);

		return entry;
	}

public:
	address operator()(const std::string_view& dll_name, const std::string_view& interface_name)
	{
		const auto& entry = Get_entry_(dll_name);
		//const auto& fn = entry.at(interface_name);

		const auto found = entry.find(interface_name);
		runtime_assert(found!=entry.end());

#ifdef CHEAT_HAVE_CONSOLE
		const auto& original_interface_name     = found.key( );
		const auto  original_interface_name_end = original_interface_name._Unchecked_end( );

		std::string msg = "Found interface: ";
		msg += interface_name;
		if (*original_interface_name_end != '\0')
		{
			msg += " (";
			msg += original_interface_name;
			msg += original_interface_name_end;
			msg += ')';
		}

		msg += " in module ";
		msg += dll_name;
		CHEAT_CONSOLE_LOG(msg);

#endif

		return std::invoke(found.value( ));
	}
};

[[maybe_unused]] static address _Get_vfunc(void* instance, size_t index)
{
	return cheat::hooks::_Pointer_to_virtual_class_table(instance)[index];
}

address csgo_interface_base::addr( ) const
{
	return result_;
}

csgo_interface_base& csgo_interface_base::operator=(const address& addr)
{
	Set_result_assert_( );
	result_ = addr;
	return *this;
}

void csgo_interface_base::Set_result_assert_( ) const
{
	(void)this;
	runtime_assert(result_ == 0u, "Result already set!");
}

#ifdef CHEAT_GUI_TEST
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
#endif

bool csgo_interfaces::load_impl( )
{
	//unused
#if 0
#ifndef CHEAT_GUI_TEST
	csgo_path = all_modules::get_ptr()->owner( ).work_dir( );
#else
	using string_type = filesystem::path::string_type;
	const auto steam_path = filesystem::path(winreg::RegKey(HKEY_CURRENT_USER, L"Software\\Valve\\Steam").GetStringValue(L"SteamPath")).make_preferred( );

	for (auto& dir: filesystem::directory_iterator(steam_path / L"steamapps" / L"common"))
	{
		const auto file = dir / L"steam_appid.txt";
		if (!exists(file))
			continue;

		std::ifstream ifs(file.native( ));
		using it = std::istreambuf_iterator<decltype(ifs)::char_type>;

		const auto str = std::string(it(ifs), it( ));

		if (str.starts_with("730") && (str.size( ) == 3 || !std::isdigit(str[3])))
		{
			csgo_path = dir;
			break;
		}
	}
	runtime_assert(!csgo_path.empty(), "Csgo path not found!");

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
		runtime_assert(handle!=0, "Unable to load library!");
	};

	load_lib(csgo_bin, L"client");

	load_lib(bin, L"engine");
	load_lib(bin, L"datacache");
	load_lib(bin, L"vstdlib");
	load_lib(bin, L"vgui2");
	load_lib(bin, L"vguimatsurface");
	load_lib(bin, L"vphysics");
	load_lib(bin, L"inputsystem");

	all_modules::get_ptr()->update(true);

#endif

#endif
#endif

#ifndef CHEAT_GUI_TEST

	// ReSharper disable once CppInconsistentNaming
	auto get_game_interface = interfaces_cache( );

	client = get_game_interface("client.dll", "VClient");
	entity_list = get_game_interface("client.dll", "VClientEntityList");
	prediction = get_game_interface("client.dll", "VClientPrediction");
	game_movement = get_game_interface("client.dll", "GameMovement");

	engine = get_game_interface("engine.dll", "VEngineClient");
	mdl_info = get_game_interface("engine.dll", "VModelInfoClient");
	mdl_render = get_game_interface("engine.dll", "VEngineModel");
	render_view = get_game_interface("engine.dll", "VEngineRenderView");
	engine_trace = get_game_interface("engine.dll", "EngineTraceClient");
	debug_overlay = get_game_interface("engine.dll", "VDebugOverlay");
	game_events = get_game_interface("engine.dll", "GAMEEVENTSMANAGER002");
	engine_sound = get_game_interface("engine.dll", "IEngineSoundClient");

	mdl_cache = get_game_interface("datacache.dll", "MDLCache");
	material_system = get_game_interface("materialsystem.dll", "VMaterialSystem");
	cvars = get_game_interface("vstdlib.dll", "VEngineCvar");
	vgui_panel = get_game_interface("vgui2.dll", "VGUI_Panel");
	vgui_surface = get_game_interface("vguimatsurface.dll", "VGUI_Surface");
	phys_props = get_game_interface("vphysics.dll", "VPhysicsSurfaceProps");
	input_sys = get_game_interface("inputsystem.dll", "InputSystemVersion");
	studio_renderer = get_game_interface("studiorender.dll", "VStudioRender");

	client_mode = _Get_vfunc(client, 10).add(5).deref(2);

	global_vars = find_signature("client.dll", "A1 ? ? ? ? 5E 8B 40 10").add(1).deref(2);
	input = find_signature("client.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10").add(1).deref(1);
	move_helper = find_signature("client.dll", "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01").add(2).deref(2);
	glow_mgr = find_signature("client.dll", "0F 11 05 ? ? ? ? 83 C8 01").add(3).deref(1);
	view_render = find_signature("client.dll", "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10").add(1).deref(1);
	weapon_sys = find_signature("client.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0").add(2).deref(1);
	local_player = find_signature("client.dll", "8B 0D ? ? ? ? 83 FF FF 74 07").add(2).deref(1);

	client_state = find_signature("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3").add(1).deref(2);

	d3d_device = find_signature("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C").add(1).deref(2);
#else
	d3d_device = (g_pd3dDevice);
#endif

	return true;
}
