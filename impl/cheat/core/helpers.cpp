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
	const auto cl_interp = m_cvar( )->FindVar(crypt_str("cl_interp"));
	const auto cl_interp_ratio = m_cvar( )->FindVar(crypt_str("cl_interp_ratio"));
	const auto sv_client_min_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_min_interp_ratio"));
	const auto sv_client_max_interp_ratio = m_cvar( )->FindVar(crypt_str("sv_client_max_interp_ratio"));
	const auto cl_updaterate = m_cvar( )->FindVar(crypt_str("cl_updaterate"));
	const auto sv_minupdaterate = m_cvar( )->FindVar(crypt_str("sv_minupdaterate"));
	const auto sv_maxupdaterate = m_cvar( )->FindVar(crypt_str("sv_maxupdaterate"));

	auto updaterate = math::clamp(cl_updaterate, sv_minupdaterate, sv_maxupdaterate);
	auto lerp_ratio = math::clamp(cl_interp_ratio, sv_client_min_interp_ratio, sv_client_max_interp_ratio);

	return math::clamp(lerp_ratio / updaterate, cl_interp, 1.0f);
#endif

#if 0

	const auto cl_interp = _Find_cvar("cl_interp");
	const auto cl_updaterate = _Find_cvar("cl_updaterate");
	const auto cl_interp_ratio = _Find_cvar("cl_interp_ratio");

	const auto a2 = cl_updaterate;
	const auto a1 = cl_interp;
	const auto v2 = cl_interp_ratio / a2;

	return fmaxf(a1, v2);

#endif

	const auto cl_interp = _Find_cvar("cl_interp");
	const auto cl_updaterate = _Find_cvar("cl_updaterate");
	const auto cl_interp_ratio = _Find_cvar("cl_interp_ratio");
	const auto sv_minupdaterate = _Find_cvar("sv_minupdaterate");
	const auto sv_maxupdaterate = _Find_cvar("sv_maxupdaterate");

	const auto update_rate = std::clamp<float>(*cl_updaterate, *sv_minupdaterate, *sv_maxupdaterate);
	float lerp_ratio = *cl_interp_ratio;

	if (lerp_ratio == 0.0f)
		lerp_ratio = 1.0f;

	const float lerp_amount = *cl_interp;

	const auto min = _Find_cvar("sv_client_min_interp_ratio");
	const auto max = _Find_cvar("sv_client_max_interp_ratio");
	(void)max;

	if (max != nullptr && min != nullptr && static_cast<float>(*min) != -1.0f)
	{
		lerp_ratio = std::clamp<float>(lerp_ratio, *min, *max);
	}
	else
	{
		if (lerp_ratio == 0.0f)
			lerp_ratio = 1.0f;
	}

	const auto ret = std::max(lerp_amount, lerp_ratio / update_rate);
	return ret;
}

float cheat::_Unlag_limit( )
{
	return *_Find_cvar("sv_maxunlag");
}

float cheat::_Unlag_range( )
{
	static auto range = 0.2f;
	return range;
}

ConVar* cheat::_Find_cvar(const string_view& cvar)
{
	struct cached_cvar
	{
		cached_cvar(ConVar* c)
		{
			BOOST_ASSERT(c!=nullptr);
			obj = c;
			addr = c;
		}

		bool valid(const string_view& name) const
		{
			if (addr != obj)
				return false;

			const auto name_size = name.size( );
			const auto& obj_name = obj->m_pszName;

			if (obj_name[name_size] != '\0')
				return false;
			if (obj_name[name_size - 1] == '\0')
				return false;
			if (name != string_view(obj_name, name_size))
				return false;
			return true;
		}

		ConVar* obj;
		address addr;
	};

	static auto cache = unordered_map<string, cached_cvar>( );
	static auto mtx = mutex( );

	const auto cvar_hash = invoke(cache.hash_function( ), cvar);

	constexpr auto get_root_cvar = []
	{
		const auto cvars = csgo_interfaces::get_ptr( )->cvars.get( );
		return address(cvars).add(0x30).deref(1).ptr<ConVar>( );
	};
	const auto get_cvar_from_game = [&]( )-> ConVar*
	{
		for (auto cv = get_root_cvar( ); cv != nullptr; cv = cv->m_pNext)
		{
			if (cv->m_pszName == cvar)
				return cv;
		}

		return nullptr;
	};

	const auto found = cache.find(cvar, cvar_hash);
	if (const auto cache_end = cache.end( ); found == cache_end)
	{
		const auto size_before = cache.size( );
		const auto lock = make_lock_guard(mtx);

		return [&]( )-> cached_cvar&
		{
			if (size_before != cache.size( ) || cache_end != cache.end( ))
				return cache.at(cvar, cvar_hash);

			auto cv = get_cvar_from_game( );
#ifdef CHEAT_HAVE_CONSOLE
			_Log_to_console(format("Cvar {} found", cvar));
#endif
			return cache.emplace(cvar, move(cv)).first.value( );
		}( ).obj;
	}

	auto& name = found.key( );
	auto& obj_cached = found.value( );

	if (!obj_cached.valid(name))
	{
		const auto lock = make_lock_guard(mtx);
		obj_cached = get_cvar_from_game( );
#ifdef CHEAT_HAVE_CONSOLE
		_Log_to_console(format("Cvar {} updated", cvar));
#endif
	}

	return obj_cached.obj;
}

static float _Interval_per_ticks( )
{
	return csgo_interfaces::get_ptr( )->global_vars->interval_per_tick;
}

size_t cheat::_Time_to_ticks(float time)
{
	return 0.5f + time / _Interval_per_ticks( );
}

float cheat::_Ticks_to_time(size_t ticks)
{
	return _Interval_per_ticks( ) * static_cast<float>(ticks);
}

address _Find_signature_impl::operator()(const memory_block& from, const string_view& sig) const
{
	return invoke(&memory_block::addr, sig.find('?') != sig.npos
									   ? from.find_block(signature<TEXT>(sig))
									   : from.find_block(signature<TEXT_AS_BYTES>(sig)));
}

address _Find_signature_impl::operator()(const string_view& dll_name, const string_view& sig) const
{
	const auto module = all_modules::get_ptr( )->find(dll_name);
	auto block = module->mem_block( );

	return invoke(*this, block, sig);
}

void* cheat::detail::_Vtable_pointer_impl(const string_view& from, const string_view& table_name)
{
	BOOST_ASSERT(!from.empty( ));

	auto& module_from = *all_modules::get_ptr( )->find(from);
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
