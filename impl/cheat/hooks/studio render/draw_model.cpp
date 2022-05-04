module;

#include <string_view>

module cheat.hooks.studio_render.draw_model;
import cheat.hooks.hook;
import cheat.csgo.interfaces.StudioRender;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

//CHEAT_HOOK_INSTANCE(studio_render, draw_model);
//
//static void* target( ) noexcept
//{
//	const nstd::mem::basic_address<void> vtable_holder = &nstd::instance_of<IStudioRender*>;
//	return vtable_holder.deref<1>( )[29];
//}
//
//struct replace
//{
//	void fn(DrawModelResults_t* results, const DrawModelInfo_t& info,
//			math::matrix3x4* bone_to_world,
//			float* flex_weights, float* flex_delayed_weights,
//			const math::vector3& model_origin, DrawModelFlags_t flags) noexcept
//	{
//		CHEAT_HOOK_CALL_ORIGINAL_MEMBER(results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
//	}
//};
//
//CHEAT_HOOK_INIT(studio_render, draw_model);

struct draw_model_impl final : draw_model, hook, hook_instance_member<draw_model_impl>
{
	draw_model_impl( )
	{
		entry_type entry;
		entry.set_target_method({&nstd::instance_of<IStudioRender*>, 29});
		entry.set_replace_method(&draw_model_impl::callback);

		this->init(std::move(entry));
	}

	void callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
		math::matrix3x4* bone_to_world,
		float* flex_weights, float* flex_delayed_weights,
		const math::vector3& model_origin, DrawModelFlags_t flags) const noexcept
	{
		call_original(results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
	}
};

std::string_view draw_model::class_name( ) const noexcept
{
	return "hooks::studio_render";
}

std::string_view draw_model::function_name( ) const noexcept
{
	return "draw_model";
}

template<>
template<>
nstd::one_instance_getter<draw_model*>::one_instance_getter(const std::in_place_index_t<0>)
	:item_(draw_model_impl::get_ptr( ))
{
}