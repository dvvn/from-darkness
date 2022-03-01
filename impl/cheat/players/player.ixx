module;

#include "player_includes.h"

export module cheat.players.player;
export import cheat.players.tick_record;
export import cheat.csgo.interfaces.C_CSPlayer;

export namespace cheat::players
{
	class player
	{
	public:
		player( ) = default;

		player(const player& other) = delete;
		player(player&& other) noexcept = default;
		player& operator=(const player& other) = delete;
		player& operator=(player&& other) noexcept = default;

		//rewrite this shit
#if 0

		struct team_info
		{
			constexpr team_info( ) = default;
			team_info(csgo::m_iTeamNum_t val);
			team_info(std::underlying_type_t<csgo::m_iTeamNum_t> val);

			csgo::m_iTeamNum_t value = csgo::m_iTeamNum_t::UNKNOWN;
			bool enemy = false;
			bool ghost = true; //dead,spectator,null pointer

			constexpr bool operator==(const team_info&) const = default;
		};

		void update(csgo::C_CSPlayer* ent, float curtime, float correct);

		csgo::C_CSPlayer* entptr = nullptr;
		std::optional<float> simtime;
		team_info team = {};
		int health = -1;
		std::optional<bool> dormant;
		std::optional<bool> dmgprotect;
		//todo: weaponfire
		//spawntime (also can be calculated manually while health changes from 0)

		struct ticks_info
		{
			size_t prev = static_cast<size_t>(-1);
			size_t current = static_cast<size_t>(-1);

			void set(size_t curr)
			{
				prev = current;
				current = curr;
			}
		};

		enum class update_state :uint8_t
		{
			IDLE
			, SILENT //updated, but not in lag compensation or unresolvable
			, NORMAL
		};

		struct
		{
			ticks_info server; //clock based
			ticks_info client; //simtime based
			update_state updated = update_state::IDLE;
		} ticks_diff;

		//--

		nstd::deque<tick_record_shared> ticks_stored;
		std::span<const tick_record_shared> ticks_stored_hittable;

		static size_t max_ticks_count( );

	private:
		void reset_ticks( );
		void update_animations(bool backup_layers);
		tick_record& store_tick( );

#endif
	};

	//using player_shared = detail::shared_holder<player>;
}
