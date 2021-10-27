#pragma once
#include "player.h"
#include "cheat/core/service.h"

namespace cheat
{
	class players_list final : public service_instance_shared<players_list>
	{
	public:
		players_list( );
		~players_list( ) override;

		void update( );

		//const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		load_result load_impl( ) noexcept override;

	private:
		std::vector<player> storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};
}
