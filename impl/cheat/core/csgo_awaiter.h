﻿#pragma once
#include "service.h"

namespace cheat
{
	class csgo_awaiter final: public service<csgo_awaiter>
							, service_maybe_skipped
	{
	protected:
		load_result load_impl( ) override;

	public:
		csgo_awaiter();

		bool game_loaded_before( ) const;

	protected:
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

	private:
		bool game_loaded_before_ = false;
		//nstd::os::frozen_threads_storage frozen_threads_{false};
	};
}