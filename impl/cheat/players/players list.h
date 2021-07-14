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

			utl::Vector      origin,   abs_origin;
			utl::QAngle      rotation, abs_rotation;
			utl::Vector      mins,     maxs;
			float            sim_time;
			utl::matrix3x4_t coordinate_frame;
		};

		using shared_type = utl::shared_ptr<player>;

		player(csgo::C_CSPlayer* ent);

		float sim_time;
		bool  dormant;
		bool  in_use;

		csgo::C_CSPlayer* ent;
	};

	enum players_filter_flags :uint8_t
	{
		null = 1 << 0,
		dormant = 1 << 1,
		dead = 1 << 2,
		alive = 1 << 3,
		ally = 1 << 4,
		enemy = 1 << 5,
		spectator = 1 << 6,
		all = []
		{
			std::underlying_type_t<players_filter_flags> val = 0;
			for (auto i = 0; i <= 6; ++i)
				val |= (1 << i);
			return val;
		}( )
	};

	using players_filter_bitflags = utl::bitflag<players_filter_flags>;

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
			void update_animations( );
			void store_tick( );
			void remove_old_ticks( );
		};

		using players_list_container = utl::vector<player_shared>;
		using players_list_container_interface = utl::vector<player::shared_type>;

		class players_filter
		{
		public:
			players_filter(const players_list_container_interface& cont, players_filter_bitflags f = all);
			players_filter(players_list_container_interface&& cont, players_filter_bitflags f = all);

			players_filter& add_flags(players_filter_bitflags f);
			players_filter  add_flags(players_filter_bitflags f) const;

			players_filter_bitflags flags( ) const;

		private:
			players_list_container_interface items__;
			players_filter_bitflags          flags__;
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

		const detail::players_filter& filter(players_filter_bitflags flags);

	protected:
		void        Load( ) override;
		utl::string Get_loaded_message( ) const override;

	private:
		detail::players_list_container             storage__;
		utl::unordered_set<detail::players_filter> filter_cache__;
	};
}
