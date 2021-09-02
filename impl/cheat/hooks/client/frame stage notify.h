#pragma once

#include "cheat/core/service.h"
#include "cheat/hooks/base.h"

namespace cheat::csgo
{
	class IBaseClientDLL;
	enum ClientFrameStage_t;
}

namespace cheat::hooks::client
{
	class frame_stage_notify final: public hook_base<frame_stage_notify, void(csgo::IBaseClientDLL::*)(csgo::ClientFrameStage_t)>
								  , service_sometimes_skipped
	{
	public:
		frame_stage_notify( );

	protected:
		nstd::address get_target_method_impl( ) const override;
		void          callback(csgo::ClientFrameStage_t stage) override;
	};
}
