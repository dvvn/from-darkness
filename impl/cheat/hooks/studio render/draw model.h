#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/IStudioRender.h"

namespace cheat::hooks::studio_render
{
	class draw_model final: public service<draw_model>,
							public decltype(_Detect_hook_holder(&csgo::IStudioRender::DrawModel))
	{
	public:
draw_model();


	protected:
		bool Do_load( ) override;
		void Callback(csgo::DrawModelResults_t* results, const csgo::DrawModelInfo_t& info,
					  utl::matrix3x4_t* bone_to_world, float* flex_weights, float* flex_delayed_weights, const utl::Vector& model_origin,
					  csgo::DrawModelFlags_t flags) override;
	};
}
