#include "config.h"

#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include "data_handmade.h"

#include "data_filler.h"

#include "cheat/core/csgo_modules.h"

#include <nstd/memory backup.h>
#include <nstd/runtime_assert_fwd.h>
#include <nstd/type name.h>

namespace cheat::csgo
{
	class C_BaseAnimating;
	class CAnimationLayer;
	struct VarMapping_t;
}

using namespace cheat::detail;
using netvars::netvars_storage;
using namespace cheat::csgo;

static netvars_storage* _Root_storage    = nullptr;
static netvars_storage* _Current_storage = nullptr;

template <typename T>
static void _Load_class( )
{
	const auto name       = nstd::type_name<T, "cheat::csgo">;
	auto&& [entry, added] = add_child_class_to_storage(*_Root_storage, name);
	runtime_assert(added == false);
	_Current_storage = std::addressof(entry.value( ));
}

namespace cheat::detail::netvars
{
	template <typename T = void>
	void add_netvar_to_storage(const std::string_view& name, int offset, const std::string_view& type_name = nstd::type_name<T>)
	{
		const auto added = add_netvar_to_storage(*_Current_storage, name, offset, type_name);
		runtime_assert(added == true);
	}
}

void netvars::store_handmade_netvars(netvars_storage& root_tree)
{
	const auto backups = std::make_tuple(nstd::memory_backup(_Root_storage, std::addressof(root_tree)),
										 nstd::memory_backup(_Current_storage));
	using namespace nstd::address_pipe;

	_Load_class<C_BaseEntity>( );
	add_netvar_to_storage<VarMapping_t>("m_InterpVarMap", 0x24);
	add_netvar_to_storage("m_BonesCache", csgo_modules::client.find_signature<"8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8">( ).add(9).deref(1).remove(8u).value( )
			  , type_utlvector(nstd::type_name<matrix3x4_t>));

	//-----

	_Load_class<C_BaseAnimating>( );
	add_netvar_to_storage("m_AnimOverlays", csgo_modules::client.find_signature<"8B 87 ? ? ? ? 83 79 04 00 8B">( ).add(2).deref(1).value( ),
				type_utlvector(nstd::type_name<CAnimationLayer>));
}
#endif
