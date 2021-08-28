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

			static auto info = all_modules::get_ptr( )->find([](std::wstring_view name)-> bool
			{
				constexpr auto dot_dll          = std::string_view(".dll");
				constexpr auto dot_dll_size     = dot_dll.size( );
				constexpr auto target_name      = Name.view( );
				constexpr auto target_name_size = target_name.size( );

				if (name.size( ) != target_name_size + dot_dll_size)
					return false;

				for (size_t i = 0; i < target_name_size; ++i)
				{
					if (name[i] != static_cast<wchar_t>(target_name[i]))
						return false;
				}

				for (size_t i = 0; i < dot_dll_size; ++i)
				{
					if (name[i + target_name_size] != static_cast<wchar_t>(dot_dll[i]))
						return false;
				}

				return true;
			}, &module_info::name);
			return info;
		}

		template <nstd::chars_cache Sig>
		nstd::address find_signature( ) const
		{
#ifdef _DEBUG
			static auto found_before = false;
			runtime_assert(!found_before);
			found_before = true;
#endif
			auto bytes = nstd::signature(Sig.view( ));
			auto ret   = this->get( )->mem_block( ).find_block(bytes);
			if (!ret.has_value( ))
			{
				CHEAT_CONSOLE_LOG("{}.dll -> signature {} not found", Name.view( ), Sig.view( ))
				return nullptr;
			}

			return ret->addr( );
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
