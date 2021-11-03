#pragma once
#include "player.h"
#include "cheat/service/include.h"

namespace cheat
{
	class players_list_impl final : public service<players_list_impl>
	{
	public:
		players_list_impl( );
		~players_list_impl( ) override;

		void update( );

		//const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		load_result load_impl( ) noexcept override;

	private:
		std::vector<player> storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};

	CHEAT_SERVICE_SHARE(players_list);
}
