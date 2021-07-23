#pragma once
#include "player.h"

#include "cheat/core/service.h"

#include "cheat/sdk/entity/C_CSPlayer.h"

namespace cheat
{
	

	class players_list;

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
		using players_list_container = utl::vector<player_shared_impl>;
		using players_list_container_interface = utl::vector<player_shared>;

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
