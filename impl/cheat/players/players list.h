#pragma once
#include "cheat/core/service.h"

#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	namespace detail
	{
		class player_shared;
	}

	class players_list;

	struct player: utl::noncopyable
	{
		struct tick_record
		{
			tick_record(player& holder);

			utl::Vector origin, abs_origin;
			utl::QAngle rotation, abs_rotation;
			utl::Vector mins, maxs;
			float sim_time;
			utl::matrix3x4_t coordinate_frame;
		};

		using shared_type = utl::shared_ptr<player>;

		player(csgo::C_CSPlayer* ent);

		float sim_time;
		bool in_use;
		csgo::C_CSPlayer* ent;

		bool dormant;
		//csgo::m_iTeamNum_t team;
		bool alive;
	};

	struct alignas(uint64_t) players_filter_flags
	{
		enum team_filter:uint8_t
		{
			ALLY=1 << 0,
			ENEMY=1 << 1,
		};
		enum team_filter_ex:uint8_t
		{
			T=1 << 0,
			CT=1 << 1,
			SPEC=1 << 2
		};

		bool alive;
		bool dormant;
		bool immune;
		utl::variant<team_filter, team_filter_ex> team;

		const uint64_t& data( ) const;

		bool operator==(const players_filter_flags& other) const;

		bool operator!=(const players_filter_flags& other) const;
	};

	namespace detail
	{
		class player_shared: player::shared_type, utl::noncopyable
		{
		public:
			friend class players_list;

			void init(csgo::C_CSPlayer* owner);

			player_shared( ) = default;
			~player_shared( );
			player_shared(player_shared&& other) noexcept;
			void operator=(player_shared&& other) noexcept;

			player::shared_type share( ) const;

			using player::shared_type::get;
			using player::shared_type::owner_before;
			using player::shared_type::use_count;
			using player::shared_type::operator bool;
			using player::shared_type::operator *;
			using player::shared_type::operator->;

			void reset( ) = delete;

			bool update_simtime( );
			void update_animations(bool simple );
			void store_tick( );
			void remove_old_ticks( );
		};

		using players_list_container = utl::vector<player_shared>;
		using players_list_container_interface = utl::vector<player::shared_type>;

		class players_filter
		{
		public:
			players_filter(const players_list_container_interface& cont, const players_filter_flags& f);
			players_filter(players_list_container_interface&& cont, const players_filter_flags& f);

			players_filter& set_flags(const players_filter_flags& f);
			players_filter set_flags(const players_filter_flags& f) const;

			const players_filter_flags& flags( ) const;

		private:
			players_list_container_interface items__;
			players_filter_flags flags__;
		};
	}
}

_STD_BEGIN
	template < >
	struct hash<cheat::detail::players_filter>
	{
		_NODISCARD size_t operator()(const cheat::detail::players_filter& val) const noexcept;
	};
	template < >
	struct equal_to<cheat::detail::players_filter>
	{
		_NODISCARD bool operator()(const cheat::detail::players_filter& a, const cheat::detail::players_filter& b) const noexcept;
	};
_STD_END

namespace cheat
{
	class players_list final: public service_shared<players_list, service_mode::async>
	{
	public:
		players_list( );

		void update( );

		const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		void Load( ) override;
		utl::string Get_loaded_message( ) const override;

	private:
		detail::players_list_container storage__;
		utl::unordered_set<detail::players_filter> filter_cache__;
	};
}
