module;

#include <string_view>

module cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_CSPlayer;
import cheat.hooks.hook;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_csplayer;

struct do_extra_bone_processing_impl final : do_extra_bone_processing, hook, hook_instance_member<do_extra_bone_processing_impl>
{
	do_extra_bone_processing_impl( )
	{
		const auto vtable_holder = csgo_modules::client.find_vtable<C_CSPlayer>( );
		const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).plus(11).deref<1>( ).divide(4);

		entry_type entry;
		entry.set_target_method({vtable_holder, index});
		entry.set_replace_method(&do_extra_bone_processing_impl::callback);

		this->init(std::move(entry));
	}

	void callback(CStudioHdr* studio_hdr, math::vector3* pos, math::quaternion* q, math::matrix3x4_aligned* bone_to_world, CBoneBitList& bone_computed, CIKContext* ik_context) const noexcept
	{
		//---
	}
};

std::string_view do_extra_bone_processing::class_name( ) const noexcept
{
	return "hooks::c_csplayer";
}

std::string_view do_extra_bone_processing::function_name( ) const noexcept
{
	return "do_extra_bone_processing";
}

template<>
template<>
nstd::one_instance_getter<do_extra_bone_processing*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(do_extra_bone_processing_impl::get_ptr( ))
{
}

