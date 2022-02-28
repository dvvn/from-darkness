module;

#include "storage_includes.h"

#include <nstd/runtime_assert.h>
#include <nstd/type name.h>

module cheat.netvars:data_handmade;
import cheat.csgo.interfaces.C_BaseAnimating;
import :type_resolve;
import :data_fill;
import cheat.csgo.modules;
import nstd.mem.address;
import nstd.mem.backup;
import cheat.tools.object_name;

//namespace cheat::csgo
//{
//	class C_BasePlayer;
//	class C_BaseAnimating;
//	struct CAnimationLayer;
//	struct VarMapping_t;
//	class matrix3x4_t;
//}

using namespace cheat;
using namespace csgo;
using netvars_impl::netvars_storage;

static netvars_storage* _Root_storage = nullptr;
static netvars_storage* _Current_storage = nullptr;

template <typename T>
static void _Load_class( )
{
	using namespace netvars_impl;
	constexpr auto name = tools::csgo_object_name<T>( );
	auto [entry, added] = add_child_class_to_storage(*_Root_storage, name);
	runtime_assert(added == false);
	_Current_storage = std::addressof(*entry);
}

namespace cheat::netvars_impl
{
	template <typename Type, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view name, int offset, [[maybe_unused]] TypeProj proj = {})
	{
		string_or_view_holder type;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		type = std::invoke(proj, nstd::type_name<Type>( ));
#endif
		[[maybe_unused]] const auto added = add_netvar_to_storage(*_Current_storage, name, offset, std::move(type));
		runtime_assert(added == true);
		return offset;
	}

	template <typename Type, class A, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view name, const nstd::mem::basic_address<A> addr, TypeProj proj = {})
	{
		return add_netvar_to_storage<Type>(name, addr.value, proj);
	}

	template <typename Type, typename TypeProj = std::identity>
	auto add_netvar_to_storage(const std::string_view name, const std::string_view offset_from, int offset, TypeProj proj = {})
	{
		using namespace std::string_view_literals;
		const auto  offset_itr = _Current_storage->find(offset_from);
		const auto offset0 = offset_itr->find("offset"sv)->get<int>( );
		return add_netvar_to_storage<Type>(name, offset0 + offset, proj);
	}
}

void netvars_impl::store_handmade_netvars(netvars_storage& root_tree)
{
	using nstd::mem::backup;
	const auto backups = std::make_tuple(backup(_Root_storage, std::addressof(root_tree)), backup(_Current_storage));
	(void)backups;

	_Load_class<C_BaseEntity>( );
	add_netvar_to_storage<VarMapping_t>("m_InterpVarMap", 0x24);
	add_netvar_to_storage<matrix3x4_t>("m_BonesCache"
									   , csgo_modules::client->find_signature("8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8").plus(9).deref<1>( ).minus(8u)
									   , type_utlvector);

	_Load_class<C_BaseAnimating>( );
	add_netvar_to_storage<CAnimationLayer>("m_AnimOverlays"
										   , csgo_modules::client->find_signature("8B 87 ? ? ? ? 83 79 04 00 8B").plus(2).deref<1>( ) //m_vecRagdollVelocity - 128
										   , type_utlvector);
	add_netvar_to_storage<float>("m_flLastBoneSetupTime"
								 , csgo_modules::client->find_signature("C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F").plus(2).deref<1>( )); //m_hLightingOrigin - 32
	add_netvar_to_storage<int>("m_iMostRecentModelBoneCounter"
							   , csgo_modules::client->find_signature("89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10").plus(2).deref<1>( )); //m_nForceBone + 4

	  //_Load_class<C_BasePlayer>( );
}
