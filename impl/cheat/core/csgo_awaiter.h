#pragma once
#include "service.h"

namespace cheat
{
	class csgo_awaiter final : public service_instance_shared<csgo_awaiter>
	{
	protected:
		load_result load_impl( ) noexcept override;

	public:
		//todo:fix
		//now this stop threadpool and main thread
		/*void after_load( ) override
		{
			if (!game_loaded_before_)
				return;
			frozen_threads_.fill( );
		}
	
		void after_reset( ) override
		{
			if (!game_loaded_before_)
				return;
			frozen_threads_.clear( );
		}*/

		bool game_loaded_before = false;
		//nstd::os::frozen_threads_storage frozen_threads_{false};
	};
}
