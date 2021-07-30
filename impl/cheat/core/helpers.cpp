#include "helpers.h"

#include "console.h"
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

	const auto ticks_count = csgo_interfaces::get_ptr( )->global_vars->ticks_count;
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
	const auto cvars = csgo_interfaces::get_ptr( )->cvars.get( );
	return address(cvars).add(0x30).deref(1).ptr<ConVar>( );
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
	return csgo_interfaces::get_ptr( )->global_vars->interval_per_tick;
}

size_t cheat::_Time_to_ticks(float time)
{
	return (0.5f + time / _Interval_per_ticks( ));
}

float cheat::_Ticks_to_time(size_t ticks)
{
	return (_Interval_per_ticks( ) * static_cast<float>(ticks));
}

address _Find_signature_impl::operator()(const mb& from, const sv& sig) const
{
	return invoke(&mb::addr, sig.find('?') != sig.npos
							 ? from.find_block(signature<TEXT>(sig))
							 : from.find_block(signature<TEXT_AS_BYTES>(sig)));
}

address _Find_signature_impl::operator()(const sv& dll_name, const sv& sig) const
{
	const auto module = all_modules::get_ptr()->find(dll_name);
	auto block = module->mem_block( );

	return invoke(*this, block, sig);
}

void* cheat::detail::_Vtable_pointer_get(const string_view& from, const string_view& table_name/*, const function<void*(csgo_interfaces*)>& preferred*/)
{
	/*if (!preferred.empty( ))
	{
		const auto ifcs = csgo_interfaces::get_ptr( );
		if (const auto ptr = preferred(ifcs.get( )); ptr != nullptr)
			return ptr;
	}*/

	BOOST_ASSERT(!from.empty( ));

	auto& module_from = *all_modules::get_ptr()->find(from);
	auto& vtables = module_from.vtables( );

	const auto& vtables_cache = vtables.get_cache( );

	if (vtables_cache.empty( ))
	{
		//precache in release mode with CHEAT_NETVARS_UPDATING define, or fuck cpu

		const auto lock = make_lock_guard(vtables);
		(void)lock;

		//maybe another thread do all the work
		if (!vtables_cache.empty( ))
			goto _LOAD;

		const auto module_checksum = module_from.check_sum( );
		BOOST_ASSERT(module_checksum != 0u);
		const auto dumps_path = filesystem::path(CHEAT_DUMPS_DIR _STRINGIZE_R(modules\)) / module_from.name_wide( ) / to_wstring(module_checksum) / L"vtables.json";

		[[maybe_unused]] const auto log_helper = [&]<typename T>(T&& msg)
		{
			_Log_to_console(format("{} vtables: {}", module_from.name( ), msg));
		};

		switch (vtables.load_from_file(dumps_path))
		{
			case success:
			case nothing:
			{
#ifdef CHEAT_HAVE_CONSOLE
				log_helper("loaded from cache");
#endif
				goto _LOAD;
			}
		}

#ifdef CHEAT_HAVE_CONSOLE
		log_helper("loading started in "
#ifdef _DEBUG
				   "slow"
#else
				   "fast"
#endif
				   " mode)"
				  );
#endif
		switch (vtables.load_from_memory( ))
		{
			case success:
			case nothing:
			{
#ifdef CHEAT_HAVE_CONSOLE
				log_helper("loading finished correctly");
#endif
#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
				if (vtables.write_to_file(dumps_path) == success)
				{
#ifdef CHEAT_HAVE_CONSOLE
					log_helper("cache created");
#endif
				}
#endif
				goto _LOAD;
			}
		}

#ifdef CHEAT_HAVE_CONSOLE
		log_helper("loading finished with errors");
#endif
		return nullptr;
	}

_LOAD:

	return vtables_cache.at(table_name).addr.ptr<void>( );
}
