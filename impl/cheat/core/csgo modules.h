#pragma once

#include "cheat/utils/memory.h"

namespace cheat::csgo_modules
{
	template <nstd::chars_cache Name>
	struct _Game_module
	{
		nstd::os::module_info* get( ) const
		{
			using namespace nstd::os;

			static auto full_name = std::wstring(Name.view( ).begin( ), Name.view( ).end( )) + L".dll";
			static auto info      = all_modules::get_ptr( )->find(&module_info::name, full_name);
			return info;
		}

		template <nstd::chars_cache Sig>
		nstd::address find_signature( ) const
		{
			static auto addr = std::invoke(utils::find_signature_impl( ), this->get( )->mem_block( ), Sig.view( ));
			return addr;
		}

		template <typename Table/*, nstd::chars_cache ...IgnoreNamespaces*/>
		Table* find_vtable( ) const
		{
			constexpr auto table_name = nstd::type_name<Table, "cheat", "csgo"/*IgnoreNamespaces...*/>( );
			static void*   ptr        = std::invoke(utils::vtable_pointer_impl( ), *this->get( ), table_name);
			return static_cast<Table*>(ptr);
		}

		template <nstd::chars_cache Ifc>
		nstd::address find_interface( ) const
		{
			static auto addr = std::invoke(*nstd::one_instance<utils::csgo_interfaces_cache_impl>( ).get_ptr( ), this->get( ), Ifc.view( ));
			return addr;
		}
	};

#define CHEAT_GAME_MODULE(_NAME_)\
	_INLINE_VAR constexpr auto _NAME_ = _Game_module<#_NAME_>( )

	CHEAT_GAME_MODULE(client);
	CHEAT_GAME_MODULE(engine);
	CHEAT_GAME_MODULE(datacache);
	CHEAT_GAME_MODULE(materialsystem);
	CHEAT_GAME_MODULE(vstdlib);
	CHEAT_GAME_MODULE(vgui2);
	CHEAT_GAME_MODULE(vguimatsurface);
	CHEAT_GAME_MODULE(vphysics);
	CHEAT_GAME_MODULE(inputsystem);
	CHEAT_GAME_MODULE(studiorender);
	CHEAT_GAME_MODULE(shaderapidx9);

#undef CHEAT_GAME_MODULE
}
