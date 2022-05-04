module;

#include <string_view>

module cheat.hooks.c_base_animating.should_skip_animation_frame;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.hooks.hook;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_base_animating;

struct should_skip_animation_frame_impl final : should_skip_animation_frame, hook, hook_instance_member<should_skip_animation_frame_impl>
{
	should_skip_animation_frame_impl( )
	{
		entry_type entry;
		entry.set_target_method(csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">( ));
		entry.set_replace_method(&should_skip_animation_frame_impl::callback);

		this->init(std::move(entry));
	}

	void callback( ) const noexcept
	{
		call_original( );
	}
};

std::string_view should_skip_animation_frame::class_name( ) const noexcept
{
	return "hooks::c_base_animating";
}

std::string_view should_skip_animation_frame::function_name( ) const noexcept
{
	return "should_skip_animation_frame";
}

template<>
template<>
nstd::one_instance_getter<should_skip_animation_frame*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(should_skip_animation_frame_impl::get_ptr( ))
{
}
