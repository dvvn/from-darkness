module;

#include <cheat/hooks/instance.h>

module cheat.hooks.studio_render.draw_model;
import cheat.csgo.interfaces.StudioRender;
import cheat.hooks.base;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

#if 0
using draw_model_base = hooks::base<void(IStudioRender::*)(DrawModelResults_t*, const DrawModelInfo_t&, matrix3x4_t*, float*, float*, const Vector&, DrawModelFlags_t)>;
struct draw_model_impl :draw_model_base
{
	draw_model_impl( )
	{
		//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).studio_renderer.vfunc(29));
		const nstd::mem::basic_address vtable_holder = IStudioRender::get_ptr( );
		this->set_target_method(vtable_holder.deref<1>( )[29]);
	}

	void callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
				  matrix3x4_t* bone_to_world,
				  float* flex_weights, float* flex_delayed_weights,
				  const Vector& model_origin, DrawModelFlags_t flags)
	{
	}
};
#endif

CHEAT_HOOK_INSTANCE(studio_render, draw_model);

static void* target( ) noexcept
{
	const nstd::mem::basic_address vtable_holder = IStudioRender::get_ptr( );
	return vtable_holder.deref<1>( )[29];
}

struct replace
{
	void fn(DrawModelResults_t* results, const DrawModelInfo_t& info,
			matrix3x4_t* bone_to_world,
			float* flex_weights, float* flex_delayed_weights,
			const Vector& model_origin, DrawModelFlags_t flags) noexcept
	{

	}
};

CHEAT_HOOK_INIT(studio_render, draw_model);