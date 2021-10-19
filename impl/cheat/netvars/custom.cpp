#include "config.h"

#ifndef CHEAT_NETVARS_DUMPER_DISABLED
#include "custom.h"

#include "data_filler.h"

#include "cheat/core/csgo modules.h"
#include "cheat/csgo/CUtlVector.hpp"

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
using namespace cheat::csgo;

static netvars_storage* _Root_storage    = nullptr;
static netvars_storage* _Current_storage = nullptr;

template <typename T>
static void _Load_class( )
{
	const auto name       = nstd::drop_namespaces(nstd::type_name<T>);
	auto&& [entry, added] = _Add_child_class(*_Root_storage, name);
	runtime_assert(added == false);
	_Current_storage = std::addressof(entry.value( ));
}

namespace cheat::detail
{
	template <typename T = void>
	static void _Save_netvar(const std::string_view& name, int offset, std::string&& type_name = std::string(nstd::type_name<T>))
	{
		const auto added = _Save_netvar(*_Current_storage, name, offset, std::move(type_name));
		runtime_assert(added == true);
	}
}

void cheat::detail::_Write_custom_netvars(netvars_storage& root_tree)
{
	const auto backups = std::make_tuple(nstd::memory_backup(_Root_storage, std::addressof(root_tree)),
										 nstd::memory_backup(_Current_storage));
	using namespace nstd::address_pipe;

	_Load_class<C_BaseEntity>( );
	_Save_netvar<VarMapping_t>("m_InterpVarMap", 0x24);
	_Save_netvar("m_BonesCache", CHEAT_FIND_SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", add(9), deref(1), remove(8u), value)
			   , _As_utlvector_type(nstd::type_name<matrix3x4_t>));

	//-----

	_Load_class<C_BaseAnimating>( );
	_Save_netvar("m_AnimOverlays", CHEAT_FIND_SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", add(2), deref(1), value),
				 _As_utlvector_type(nstd::type_name<CAnimationLayer>));
}
#endif
