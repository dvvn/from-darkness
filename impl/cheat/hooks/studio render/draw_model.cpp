module;

#include <cheat/hooks/instance.h>

module cheat.hooks.studio_render.draw_model;
import cheat.csgo.interfaces.StudioRender;
import cheat.hooks.base;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(studio_render, draw_model);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = &nstd::instance_of<IStudioRender*>;
	return vtable_holder.deref<1>( )[29];
}

struct replace
{
	void fn(DrawModelResults_t* results, const DrawModelInfo_t& info,
			math::matrix3x4* bone_to_world,
			float* flex_weights, float* flex_delayed_weights,
			const math::vector3& model_origin, DrawModelFlags_t flags) noexcept
	{
		CHEAT_HOOK_CALL_ORIGINAL_MEMBER(results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
	}
};

CHEAT_HOOK_INIT(studio_render, draw_model);