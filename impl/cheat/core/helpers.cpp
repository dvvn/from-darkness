#include "helpers.h"

#include "csgo interfaces.h"

#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/ICvar.hpp"
#include "cheat/utils/signature.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;
using namespace utl::detail;

float cheat::_Lerp_time( )
{
#if 0
	static auto cl_interp = m_cvar( )->FindVar(crypt_str("cl_interp"));
	static auto cl_interp_ratio = m_cvar( )->FindVar(crypt_str("cl_interp_ratio"));
	static auto sv_client_min_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_min_interp_ratio"));
	static auto sv_client_max_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_max_interp_ratio"));
	static auto cl_updaterate = m_cvar( )->FindVar(crypt_str("cl_updaterate"));
	static auto sv_minupdaterate = m_cvar( )->FindVar(crypt_str("sv_minupdaterate"));
	static auto sv_maxupdaterate = m_cvar( )->FindVar(crypt_str("sv_maxupdaterate"));

	auto updaterate = math::clamp(cl_updaterate->GetFloat( ), sv_minupdaterate->GetFloat( ), sv_maxupdaterate->GetFloat( ));
	auto lerp_ratio = math::clamp(cl_interp_ratio->GetFloat( ), sv_client_min_interp_ratio->GetFloat( ), sv_client_max_interp_ratio->GetFloat( ));

	return math::clamp(lerp_ratio / updaterate, cl_interp->GetFloat( ), 1.0f);
#endif

#if 0

	static auto cl_interp = _Find_cvar("cl_interp");
	static auto cl_updaterate = _Find_cvar("cl_updaterate");
	static auto cl_interp_ratio = _Find_cvar("cl_interp_ratio");

	const auto a2 = cl_updaterate->GetFloat( );
	const auto a1 = cl_interp->GetFloat( );
	const auto v2 = cl_interp_ratio->GetFloat( ) / a2;

	return fmaxf(a1, v2);

#endif

	constexpr auto get_lerp_time = []
	{
		static auto cl_interp = _Find_cvar("cl_interp");
		static auto cl_updaterate = _Find_cvar("cl_updaterate");
		static auto cl_interp_ratio = _Find_cvar("cl_interp_ratio");
		static auto sv_minupdaterate = _Find_cvar("sv_minupdaterate");
		static auto sv_maxupdaterate = _Find_cvar("sv_maxupdaterate");

		const auto update_rate = std::clamp(cl_updaterate->GetFloat( ), sv_minupdaterate->GetFloat( ), sv_maxupdaterate->GetFloat( ));
		auto lerp_ratio = cl_interp_ratio->GetFloat( );

		if (lerp_ratio == 0.0f)
			lerp_ratio = 1.0f;

		const auto lerp_amount = cl_interp->GetFloat( );

		static auto min = _Find_cvar("sv_client_min_interp_ratio");
		static auto max = _Find_cvar("sv_client_max_interp_ratio");
		(void)max;

		if (max && min && min->GetFloat( ) != -1.0f)
		{
			lerp_ratio = std::clamp(lerp_ratio, min->GetFloat( ), max->GetFloat( ));
		}
		else
		{
			if (lerp_ratio == 0.0f)
				lerp_ratio = 1.0f;
		}

		const auto ret = std::max(lerp_amount, lerp_ratio / update_rate);
		return ret;
	};

	const auto ticks_count = csgo_interfaces::get_shared( )->global_vars->ticks_count;
	static auto last_ticks_count = ticks_count - 1;

	static float value_cached;

	if (ticks_count != last_ticks_count)
	{
		last_ticks_count = ticks_count;
		value_cached = get_lerp_time( );
	}

	(void)last_ticks_count;

	return value_cached;
}

float cheat::_Unlag_limit( )
{
	static auto max_unlag = _Find_cvar("sv_maxunlag");
	return max_unlag->GetFloat( );
}

float cheat::_Unlag_range( )
{
	static auto range = 0.2f;
	return range;
}

static ConVar* _Get_root_cvar( )
{
	const auto cvars = csgo_interfaces::get_shared( )->cvars.get( );
	return address(cvars).add(0x30).deref(1).raw<ConVar>( );
	//return *(ConVar**)((uintptr_t)cvars + 0x30);
};

ConVar* cheat::_Find_cvar(const string_view& cvar)
{
	for (auto cv = _Get_root_cvar( ); cv != nullptr; cv = cv->m_pNext)
	{
		if (cv->m_pszName == cvar)
			return cv;
	}
	return nullptr;
}

static float _Interval_per_ticks( )
{
	return csgo_interfaces::get_shared( )->global_vars->interval_per_tick;
}

size_t cheat::_Time_to_ticks(float time)
{
	return (0.5f + time / _Interval_per_ticks( ));
}

float cheat::_Ticks_to_time(size_t ticks)
{
	return (_Interval_per_ticks( ) * ticks);
}

address _Find_signature_impl::operator()(const mb& from, const sv& sig) const
{
	return invoke(&mb::addr, sig.find('?') != sig.npos
							 ? from.find_block(signature<TEXT>(sig))
							 : from.find_block(signature<TEXT_AS_BYTES>(sig)));
}

address _Find_signature_impl::operator()(const sv& dll_name, const sv& sig) const
{
	const auto module = all_modules::get( ).find(dll_name);
	auto block = module->mem_block( );

	return invoke(*this, block, sig);
}

void* cheat::detail::_Vtable_pointer_get(const string_view& from, const string_view& table_name, const function<void*(csgo_interfaces*)>& preferred)
{
	if (!preferred.empty( ))
	{
		const auto ifcs = csgo_interfaces::get_shared( );
		if (const auto ptr = preferred(ifcs.get( )); ptr != nullptr)
			return ptr;
	}

	constexpr auto load_vtables = [](module_info& module_from)-> bool
	{
		//precache in release mode with CHEAT_NETVARS_UPDATING define, or fuck cpu

		const auto module_checksum = module_from.check_sum( );
		BOOST_ASSERT(module_checksum != 0u);
		auto dumps_path = filesystem::path(CHEAT_DUMPS_DIR _STRINGIZE_R(modules\)) / module_from.name_wide( ) / to_wstring(module_checksum) / L"vtables.json";
#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
		(void)dumps_path;
#else
		if (!exists(dumps_path))
			dumps_path.clear( );//prefer to load from memory in release mode
#endif

		const auto load_result = module_from.vtables( ).load(dumps_path);
		BOOST_ASSERT(load_result != error);
		return load_result == success;
	};

	const auto table_name_hash = invoke(make_default<decltype(std::declval<vtables_storage>( ).get_cache( ).hash_function( ))>( ), table_name);

	const auto check_specific_module = [&]( )-> void*
	{
		auto& info = *all_modules::get( ).find(from);
		const auto& vtables = info.vtables( );
		const auto& vtables_cache = vtables.get_cache( );

		if (vtables_cache.empty( ))
		{
			[[maybe_unused]] const auto loaded = load_vtables(info);
			BOOST_ASSERT(loaded==true);
		}

		return vtables_cache.at(table_name, table_name_hash).addr.raw<void>( );
	};
	const auto check_all_modules = [&]( )-> void*
	{
		static constexpr auto to_str_lower = [](const wstring_view& str)
		{
			auto tmp = basic_string(str);
			ranges::transform(tmp, tmp.begin( ), towlower);
			return tmp;
		};

		static const auto csgo_dir = to_str_lower(all_modules::get( ).owner( ).work_dir( ));

		for (auto&& info: all_modules::get( ).all( ) | ranges::views::transform([](module_info& i) { return addressof(i); }))
		{
			auto& vtables_cache = info->vtables( ).get_cache( );

			// ReSharper disable once CppTooWideScopeInitStatement
			const auto class_contains_vtables = [&]( )-> bool
			{
				if (!vtables_cache.empty( ))
					return true;

				static unordered_set<module_info*> uselles_infos;
				if (uselles_infos.contains(info))
					return false;

				const auto dir_sv = info->work_dir( );

				const auto wrong_dir = [&]
				{
					if (dir_sv.size( ) <= csgo_dir.size( ))
						return true;
					if (to_str_lower(dir_sv).starts_with(csgo_dir) == false)
						return true;

					return false;
				};

				// ReSharper disable once CppTooWideScopeInitStatement
				const auto no_vtables = [&]
				{
					return load_vtables(*info) == false;
				};

				if (wrong_dir( ) || no_vtables( ))
				{
					uselles_infos.insert(info);
					return false;
				}

				return true;
			};

			if (!class_contains_vtables( ))
				continue;
			const auto found = vtables_cache.find(table_name, table_name_hash);
			if (found == vtables_cache.end( ))
				continue;

			return found.value( ).addr.raw<void>( );
		}

		return nullptr;
	};

	return from.empty( )
		   ? check_all_modules( )
		   : check_specific_module( );
}
