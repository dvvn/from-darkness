#pragma once

#include "cheat/core/service.h"
#include "cheat/sdk/IStudioRender.h"

namespace cheat::hooks::studio_render
{
	class draw_model final: public service<draw_model>
						  , public decltype(_Detect_hook_holder(&csgo::IStudioRender::DrawModel))
						  , service_hook_helper
#ifdef CHEAT_GUI_TEST
						  , service_always_skipped
#endif
	{
	protected:
		utl::address get_target_method_impl( ) const override;
		void         callback(csgo::DrawModelResults_t* results, const csgo::DrawModelInfo_t& info,
							  csgo::matrix3x4_t*        bone_to_world, float*                 flex_weights, float* flex_delayed_weights, const csgo::Vector& model_origin,
							  csgo::DrawModelFlags_t    flags) override;
	};
}
