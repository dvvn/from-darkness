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
	struct draw_model_impl final : service<draw_model_impl>
							, dhooks::_Detect_hook_holder_t<__COUNTER__
														  , void(csgo::IStudioRender::*)(csgo::DrawModelResults_t*, const csgo::DrawModelInfo_t&, csgo::matrix3x4_t*, float*
																					   , float*
																					   , const csgo::Vector&, csgo::DrawModelFlags_t)>
	{
		draw_model_impl( );

	protected:
		load_result load_impl( ) noexcept override;
		void* get_target_method( ) const override;
		void callback(csgo::DrawModelResults_t* results,
					  const csgo::DrawModelInfo_t& info,
					  csgo::matrix3x4_t* bone_to_world,
					  float* flex_weights,
					  float* flex_delayed_weights,
					  const csgo::Vector& model_origin,
					  csgo::DrawModelFlags_t flags) override;
	};

	CHEAT_SERVICE_SHARE(draw_model);
}
