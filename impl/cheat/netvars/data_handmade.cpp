#include "data_handmade.h"
#include "data_filler.h"

#include "cheat/core/csgo_modules.h"

#include <nstd/mem/backup.h>
#include <nstd/runtime_assert_fwd.h>
#include <nstd/type name.h>

namespace cheat::csgo
{
	class C_BasePlayer;
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
	template <typename Type, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view& name, int offset, TypeProj&& type_proj = {})
	{
		string_or_view_holder type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		type = std::invoke(type_proj, nstd::type_name<Type>);
#endif
		const auto added = add_netvar_to_storage(*_Current_storage, name, offset, std::move(type));
		runtime_assert(added == true);
		return offset;
	}

	template <typename Type, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view& name, nstd::address addr, TypeProj&& type_proj = {})
	{
		return add_netvar_to_storage<Type>(name, addr.value( ), std::forward<TypeProj>(type_proj));
	}

	template <typename Type, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view& name, const std::string_view& offset_from, int offset, TypeProj&& type_proj = {})
	{
		using namespace std::string_view_literals;
		const auto offset0 = _Current_storage->find(offset_from)->find("offset"sv)->get<int>( );
		return add_netvar_to_storage<Type>(name, offset0 + offset, std::forward<TypeProj>(type_proj));
	}
}

void netvars::store_handmade_netvars(netvars_storage& root_tree)
{
	using nstd::mem::backup;
	const auto backups = std::make_tuple(backup(_Root_storage, std::addressof(root_tree)), backup(_Current_storage));
	(void)backups;

	_Load_class<C_BaseEntity>( );
	add_netvar_to_storage<VarMapping_t>("m_InterpVarMap", 0x24);
	add_netvar_to_storage<matrix3x4_t>("m_BonesCache"
									 , csgo_modules::client->find_signature("8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8").add(9).deref(1).remove(8u)
									 , type_utlvector);

	_Load_class<C_BaseAnimating>( );
	add_netvar_to_storage<CAnimationLayer>("m_AnimOverlays"
										 , csgo_modules::client->find_signature("8B 87 ? ? ? ? 83 79 04 00 8B").add(2).deref(1) //m_vecRagdollVelocity - 128
										 , type_utlvector);
	add_netvar_to_storage<float>("m_flLastBoneSetupTime"
							   , csgo_modules::client->find_signature("C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F").add(2).deref(1)); //m_hLightingOrigin - 32
	add_netvar_to_storage<int>("m_iMostRecentModelBoneCounter"
							 , csgo_modules::client->find_signature("89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10").add(2).deref(1)); //m_nForceBone + 4

	//_Load_class<C_BasePlayer>( );
}
