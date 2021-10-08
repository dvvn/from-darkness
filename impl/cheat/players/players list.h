#pragma once
#include "player.h"
#include "cheat/netvars/config.h"

#include "cheat/features/base.h"

#if CHEAT_MODE_INGAME && __has_include("cheat/sdk/generated/C_BasePlayer_h") && __has_include("cheat/sdk/generated/C_BaseAnimating_h")
#define CHEAT_FEATURE_PLAYER_LIST 1
#else
#define CHEAT_FEATURE_PLAYER_LIST 0
#endif

namespace cheat
{
	class players_list final : public features::service_feature<players_list>
	{
	public:
		players_list();
		~players_list() override;

		void update();

		//const detail::players_filter& filter(const players_filter_flags& flags);
		
	protected:
		load_result load_impl() noexcept override;

	private:
		struct storage_type;
		std::unique_ptr<storage_type> storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};
}
