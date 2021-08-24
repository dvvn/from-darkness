#include "memory.h"

using namespace cheat::utils;
using namespace cheat::utils::detail;

nstd::address find_signature_impl::operator()(const nstd::memory_block& from, const std::string_view& sig) const
{
	using namespace nstd;
	return std::invoke(&memory_block::addr, sig.find('?') != sig.npos
												? from.find_block(signature<signature_parse_mode::TEXT>(sig))
												: from.find_block(signature<signature_parse_mode::TEXT_AS_BYTES>(sig)));
}

nstd::address find_signature_impl::operator()(const std::string_view& dll_name, const std::string_view& sig) const
{
	const auto module = nstd::os::all_modules::get_ptr( )->find(dll_name);
	auto       block  = module->mem_block( );

	return std::invoke(*this, block, sig);
}

void* vtable_pointer_impl::operator()(const std::string_view& from, const std::string_view& table_name) const
{
	runtime_assert(!from.empty( ));

	auto& module_from = *nstd::os::all_modules::get_ptr( )->find(from);
	auto& vtables     = module_from.vtables( );

	const auto& vtables_cache = vtables.get_cache( );

	if (vtables_cache.empty( ))
	{
		//precache in release mode with CHEAT_NETVARS_UPDATING define, or fuck cpu

		const auto lock = std::lock_guard(vtables);
		(void)lock;

		//maybe another thread do all the work
		if (!vtables_cache.empty( ))
			goto _LOAD;

		const auto module_checksum = module_from.check_sum( );
		runtime_assert(module_checksum != 0u);
		const auto dumps_path = std::filesystem::path(CHEAT_DUMPS_DIR NSTD_RAW(modules\)) / module_from.name_wide( ) / std::to_wstring(module_checksum) / L"vtables.json";

#ifdef CHEAT_HAVE_CONSOLE
#ifdef _DEBUG
#define CHEAT_VTABLES_DUMP_SPEED "SLOW"
#else
#define CHEAT_VTABLES_DUMP_SPEED "FAST"
#endif

#define CHEAT_CONSOLE_LOG_HELPER(...)\
	CHEAT_CONSOLE_LOG("{} vtables: {}", module_from.name( ), __VA_ARGS__)
#else
#define CHEAT_CONSOLE_LOG_HELPER(...) (void)0
#endif

		CHEAT_CONSOLE_LOG_HELPER(std::ostringstream( )
								 << "Trying to load from "
								 << (exists(dumps_path) ? "CACHE" : "MEMORY")
								 << " in "
								 << CHEAT_VTABLES_DUMP_SPEED
								 << " mode");

		try
		{
			//#ifdef CHEAT_HAVE_CONSOLE
			//			auto timer = utl::timer( );
			//			timer.set_start( );
			//#endif

			if (vtables.load(dumps_path))
			{
				CHEAT_CONSOLE_LOG_HELPER("Loaded");
				goto _LOAD;
			}
			CHEAT_CONSOLE_LOG_HELPER("Not loaded");
		}
		catch (const std::exception& ex)
		{
			CHEAT_CONSOLE_LOG_HELPER(std::format("Not loaded: ", ex.what( )));
		}

		return nullptr;
	}

_LOAD:
	return vtables_cache.at(table_name).addr.ptr<void>( );
}
