module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.studio_render:draw_model;
import cheat.hooks.base;
import cheat.csgo.interfaces;

//#include "cheat/hooks/base.h"
//
//namespace cheat::csgo
//{
//	struct DrawModelResults_t;
//	struct DrawModelInfo_t;
//	enum class DrawModelFlags_t;
//	class Vector;
//	class matrix3x4_t;
//	class IStudioRender;
//}

export namespace cheat::hooks::studio_render
{
	struct draw_model final : hook_base<draw_model
		, void(csgo::IStudioRender::*)(csgo::DrawModelResults_t*, const csgo::DrawModelInfo_t&, csgo::matrix3x4_t*, float*
									   , float*
									   , const csgo::Vector&, csgo::DrawModelFlags_t)>

	{
		draw_model( );

	protected:
		void* get_target_method( ) const override;
		void callback(csgo::DrawModelResults_t* results,
					  const csgo::DrawModelInfo_t& info,
					  csgo::matrix3x4_t* bone_to_world,
					  float* flex_weights,
					  float* flex_delayed_weights,
					  const csgo::Vector& model_origin,
					  csgo::DrawModelFlags_t flags) override;
	};
}
