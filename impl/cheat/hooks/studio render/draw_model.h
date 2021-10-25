#pragma once

#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	struct DrawModelResults_t;
	struct DrawModelInfo_t;
	enum class DrawModelFlags_t;
	class Vector;
	class matrix3x4_t;
	class IStudioRender;
}

namespace cheat::hooks::studio_render
{
	struct draw_model final :
			hook_instance_shared<draw_model
								,__COUNTER__
							   , void(csgo::IStudioRender::*)(csgo::DrawModelResults_t*, const csgo::DrawModelInfo_t&, csgo::matrix3x4_t*, float*, float*
															, const csgo::Vector&, csgo::DrawModelFlags_t)>
	{
		draw_model( );

	protected:
		load_result load_impl( ) noexcept override;
		nstd::address get_target_method_impl( ) const override;
		void callback(csgo::DrawModelResults_t* results,
					  const csgo::DrawModelInfo_t& info,
					  csgo::matrix3x4_t* bone_to_world,
					  float* flex_weights,
					  float* flex_delayed_weights,
					  const csgo::Vector& model_origin,
					  csgo::DrawModelFlags_t flags) override;
	};
}
