#pragma once
#include "tick record.h"

#include "cheat/sdk/entity/C_BaseEntity.h"

#include <include/veque.hpp>

#include <memory>
#include <span>

namespace cheat
{
	namespace csgo
	{
		// ReSharper disable once CppInconsistentNaming
		class C_CSPlayer;
	}

	struct player: detail::shared_holder_info
	{
		csgo::C_CSPlayer* ent;

		float              sim_time;
		bool               dormant;
		csgo::m_iTeamNum_t team;
		bool               alive;

		using ticks_storage = veque::veque<detail::tick_record_shared_impl>;

		ticks_storage                        ticks;
		std::span<ticks_storage::value_type> ticks_window;

		static size_t max_ticks_count( );
	};

	using player_shared = std::shared_ptr<player>;

	namespace detail
	{
		class player_shared_impl final: public shared_holder<player>
		{
		public:
			void init(csgo::C_CSPlayer* owner);

			bool update_simtime( );
			void update_animations(bool simple);
			void store_tick( );
			void remove_old_ticks(float curtime);
		};
	}
}
