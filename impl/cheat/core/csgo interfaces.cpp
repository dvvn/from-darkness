#include "csgo interfaces.h"

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

class interfaces_cache: public one_instance_shared<interfaces_cache>
{
	using entry_type = unordered_map<string_view, InstantiateInterfaceFn>;
	unordered_map<module_info*, entry_type> cache__;

	struct
	{
		string_view module_name;
		entry_type* entry = nullptr;
		//-
	} last_used__;

	entry_type& Get_entry_(const string_view& dll_name)
	{
		if (last_used__.module_name != dll_name)
		{
			const auto info = all_modules::get( ).find(dll_name);

			auto& entry = cache__[info];
			if (entry.empty( ))
				Fill_entry_(entry, info);

			last_used__.module_name = dll_name;
			last_used__.entry = addressof(entry);
		}
		return *last_used__.entry;
	}

	static void Fill_entry_(entry_type& entry, module_info* info)
	{
		BOOST_ASSERT_MSG(entry.empty(), "Entry already filled!");

		auto& exports = info->exports( );

		auto load_result = exports.load( );
		(void)load_result;
		BOOST_ASSERT_MSG(load_result != error, "Unable to load exports");

		const auto& create_fn = exports.get_cache( ).at("CreateInterface");
		const auto  reg = create_fn.rel32(0x5).add(0x6).deref(2).raw<CInterfaceRegister>( );

		auto temp_entry = vector<decltype(cache__)::mapped_type::value_type>( );
		for (auto r = reg; r != nullptr; r = r->next)
			temp_entry.emplace_back(make_pair(string_view(r->name), r->create_fn));

		const auto contains_duplicate = [&](const string_view& new_string, size_t original_size)
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
		const auto drop_underline = [&](const string_view& str, size_t original_size) -> optional<string_view>
		{
			if (str.ends_with('_'))
			{
				if (const auto str2 = string_view(str.begin( ), str.end( ) - 1); !contains_duplicate(str2, original_size))
					return str2;
			}
			return { };
		};
		const auto get_pretty_string = [&](const string_view& str) -> optional<string_view>
		{
			size_t remove = 0;
			for (auto c: str | ranges::views::reverse)
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
	}

public:
	void* load(const string_view& dll_name, const string_view& interface_name)
	{
		auto& entry = Get_entry_(dll_name);
		auto& fn = entry.at(interface_name);
		return fn( );
	}
};

address csgo_interface_base::addr( ) const
{
	return result_;
}

void csgo_interface_base::from_interface(const string_view& dll_name, const string_view& interface_name)
{
	Set_result_assert_( );
	result_ = interfaces_cache::get( ).load(dll_name, interface_name);
}

void csgo_interface_base::from_sig(const memory_block& from, const string_view& sig, size_t add, size_t deref)
{
	Set_result_assert_( );
	address found;
	if (sig.find('?') != sig.npos)
		found = from.find_block(signature<TEXT>(sig)).addr( );
	else
		found = from.find_block(signature<TEXT_AS_BYTES>(sig)).addr( );
	result_ = found.add(add).deref_safe(deref);
}

void csgo_interface_base::from_sig(const string_view& dll_name, const string_view& sig, size_t add, size_t deref)
{
	const auto info = all_modules::get( ).find(dll_name);
	return from_sig(info->mem_block( ), sig, add, deref);
}

void csgo_interface_base::from_vfunc(void* instance, size_t index, size_t add, size_t deref)
{
	Set_result_assert_( );
	auto info = hooks::method_info::make_member_virtual(instance, index);
	if (info.update( ))
	{
		const address fn = info.get( );
		result_ = fn.add(add).deref_safe(deref);
	}
}

void csgo_interface_base::from_ptr(void* ptr)
{
	Set_result_assert_( );
	result_ = ptr;
}

void csgo_interface_base::Set_result_assert_( ) const
{
	(void)this;
	BOOST_ASSERT_MSG(result_ == 0u, "Result already set!");
}

csgo_interfaces::~csgo_interfaces( )
{
}

csgo_interfaces::csgo_interfaces( )
{
}

#ifdef CHEAT_GUI_TEST
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
#endif

void csgo_interfaces::Load( )
{
	//unused
#if 0
#ifndef CHEAT_GUI_TEST
	csgo_path = all_modules::get( ).owner( ).work_dir( );
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

		const auto str = string(it(ifs), it( ));

		if (str.starts_with("730") && (str.size( ) == 3 || !std::isdigit(str[3])))
		{
			csgo_path = dir;
			break;
		}
	}
	BOOST_ASSERT_MSG(!csgo_path.empty(), "Csgo path not found!");

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
		BOOST_ASSERT_MSG(handle!=0, "Unable to load library!");
	};

	load_lib(csgo_bin, L"client");

	load_lib(bin, L"engine");
	load_lib(bin, L"datacache");
	load_lib(bin, L"vstdlib");
	load_lib(bin, L"vgui2");
	load_lib(bin, L"vguimatsurface");
	load_lib(bin, L"vphysics");
	load_lib(bin, L"inputsystem");

	all_modules::get( ).update(true);

#endif

#endif
#endif

#ifndef CHEAT_GUI_TEST

	auto cache_lock = interfaces_cache::get_shared( );
	(void)cache_lock;

	client.from_interface("client.dll", "VClient");
	entity_list.from_interface("client.dll", "VClientEntityList");
	prediction.from_interface("client.dll", "VClientPrediction");
	game_movement.from_interface("client.dll", "GameMovement");

	engine.from_interface("engine.dll", "VEngineClient");
	mdl_info.from_interface("engine.dll", "VModelInfoClient");
	mdl_render.from_interface("engine.dll", "VEngineModel");
	render_view.from_interface("engine.dll", "VEngineRenderView");
	engine_trace.from_interface("engine.dll", "EngineTraceClient");
	debug_overlay.from_interface("engine.dll", "VDebugOverlay");
	game_events.from_interface("engine.dll", "GAMEEVENTSMANAGER002");
	engine_sound.from_interface("engine.dll", "IEngineSoundClient");

	mdl_cache.from_interface("datacache.dll", "MDLCache");
	material_system.from_interface("materialsystem.dll", "VMaterialSystem");
	cvars.from_interface("vstdlib.dll", "VEngineCvar");
	vgui_panel.from_interface("vgui2.dll", "VGUI_Panel");
	vgui_surface.from_interface("vguimatsurface.dll", "VGUI_Surface");
	phys_props.from_interface("vphysics.dll", "VPhysicsSurfaceProps");
	input_sys.from_interface("inputsystem.dll", "InputSystemVersion");

	client_mode.from_vfunc(client, 10, 0x5, 2);

	global_vars.from_sig("client.dll", "A1 ? ? ? ? 5E 8B 40 10", 1, 2);
	input.from_sig("client.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10", 1, 1);
	move_helper.from_sig("client.dll", "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01", 2, 2);
	glow_mgr.from_sig("client.dll", "0F 11 05 ? ? ? ? 83 C8 01", 3, 1);
	view_render.from_sig("client.dll", "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10", 1, 1);
	weapon_sys.from_sig("client.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0", 2, 1);
	local_player.from_sig("client.dll", "8B 0D ? ? ? ? 83 FF FF 74 07", 2, 1);

	client_state.from_sig("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3", 1, 2);

	d3d_device.from_sig("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C", 1, 2);
#else
	d3d_device.from_ptr(g_pd3dDevice);
#endif
}
