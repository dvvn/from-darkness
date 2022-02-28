module;

#include "cheat/hooks/base_includes.h"

export module cheat.hooks.studio_render:draw_model;
import cheat.hooks.base;
import cheat.csgo.interfaces.StudioRender;

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

namespace cheat::hooks::studio_render
{
	export class draw_model final :public hook_base<draw_model
		, void(csgo::IStudioRender::*)(csgo::DrawModelResults_t*, const csgo::DrawModelInfo_t&, csgo::matrix3x4_t*, float*
									   , float*
									   , const csgo::Vector&, csgo::DrawModelFlags_t)>

	{
	public:
		draw_model( );

	protected:
		void construct( ) noexcept override;
		void callback(csgo::DrawModelResults_t* results,
					  const csgo::DrawModelInfo_t& info,
					  csgo::matrix3x4_t* bone_to_world,
					  float* flex_weights,
					  float* flex_delayed_weights,
					  const csgo::Vector& model_origin,
					  csgo::DrawModelFlags_t flags) override;
	};
}
