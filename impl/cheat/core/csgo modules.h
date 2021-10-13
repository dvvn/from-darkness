#pragma once

#include "nstd/address.h"
#include "nstd/chars cache.h"
#include "nstd/type name.h"

namespace nstd
{
	class address;

	namespace os
	{
		class module_info;
	}
}

namespace cheat::csgo_modules
{
	namespace detail
	{
		nstd::os::module_info* get_module_impl(const std::string_view& target_name);
		nstd::address find_signature_impl(nstd::os::module_info* md, const std::string_view& sig);
		nstd::address find_csgo_interface(nstd::os::module_info* from, const std::string_view& target_name);
		void* find_vtable_pointer(nstd::os::module_info* from, const std::string_view& class_name);
	}

	template <nstd::chars_cache Name>
	struct game_module_base
	{
		nstd::os::module_info* get() const
		{
			using namespace nstd::os;

			static auto info = detail::get_module_impl(Name.view( ));
			return info;
		}

		template <nstd::chars_cache Sig>
		nstd::address find_signature() const
		{
#ifdef _DEBUG
			static auto found_before = false;
			if (found_before)
				throw;
			found_before = true;
#endif
			return detail::find_signature_impl(get( ), Sig.view( ));
		}

		template <typename Table/*, nstd::chars_cache ...IgnoreNamespaces*/>
		Table* find_vtable() const
		{
			constexpr auto table_name = nstd::type_name<Table, "cheat::csgo"/*IgnoreNamespaces...*/>;
			static void* ptr          = detail::find_vtable_pointer(this->get( ), table_name);
			return static_cast<Table*>(ptr);
		}

		template <nstd::chars_cache Ifc>
		nstd::address find_game_interface() const
		{
			static auto addr = detail::find_csgo_interface(this->get( ), Ifc.view( ));
			return addr;
		}
	};

#define CHEAT_GAME_MODULE(_NAME_)\
	_INLINE_VAR constexpr auto _NAME_ = game_module_base<#_NAME_>( )

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

#define CHEAT_FIND_SIG(_HOLDER_,_SIG_,...) \
	nstd::apply_address_pipe( cheat::csgo_modules::_HOLDER_.find_signature<_SIG_>( ), ##__VA_ARGS__ )
#define CHEAT_FIND_VTABLE(_HOLDER_,_CLASS_) \
	cheat::csgo_modules::_HOLDER_.find_vtable<_CLASS_>( )
#define CHEAT_FIND_GAME_INTERFACE(_HOLDER_,_CLASS_,...) \
	nstd::apply_address_pipe(cheat::csgo_modules::_HOLDER_.find_game_interface<_CLASS_>( ))
