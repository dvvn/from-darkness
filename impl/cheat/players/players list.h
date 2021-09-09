#pragma once
#include "player.h"

#include "cheat/core/service.h"

//#include <variant>

namespace cheat
{
	class players_list;
	//deprecated
	/*
		struct alignas(uint64_t) players_filter_flags
		{
			struct team_filter final
			{
				enum value_type :uint8_t
				{
					ALLY = 1 << 0,
					ENEMY = 1 << 1,
				};
	
				NSTD_ENUM_STRUCT_BITFLAG(team_filter)
			};
	
			struct team_filter_ex final
			{
				enum value_type :uint8_t
				{
					T = 1 << 0,
					CT = 1 << 1,
					SPEC = 1 << 2
				};
	
				NSTD_ENUM_STRUCT_BITFLAG(team_filter_ex)
			};
	
			bool alive;
			bool dormant;
			bool immune;
			std::variant<team_filter,
						 team_filter_ex> team;
	
			const uint64_t& data( ) const;
	
			bool operator==(const players_filter_flags& other) const;
			bool operator!=(const players_filter_flags& other) const;
		};
		
		namespace detail
		{
			using players_list_container = std::vector<player_shared_impl>;
			using players_list_container_interface = std::vector<player_shared>;
	
			class players_filter
			{
			public:
				players_filter(const players_list_container_interface& cont, const players_filter_flags& f);
				players_filter(players_list_container_interface&& cont, const players_filter_flags& f);
	
				players_filter& set_flags(const players_filter_flags& f);
				players_filter  set_flags(const players_filter_flags& f) const;
	
				const players_filter_flags& flags( ) const;
	
			private:
				players_list_container_interface items__;
				players_filter_flags             flags__;
			};
		}*/
}

//_STD_BEGIN
//	template < >
//	struct hash<cheat::detail::players_filter>
//	{
//		_NODISCARD size_t operator()(const cheat::detail::players_filter& val) const noexcept;
//	};
//	template < >
//	struct equal_to<cheat::detail::players_filter>
//	{
//		_NODISCARD bool operator()(const cheat::detail::players_filter& a, const cheat::detail::players_filter& b) const noexcept;
//	};
//_STD_END

namespace cheat
{
	class players_list final: public service<players_list>
							, service_maybe_skipped
	{
	public:
		players_list( );
		~players_list( ) override;

		void update( );

		//const detail::players_filter& filter(const players_filter_flags& flags);

	protected:
		load_result load_impl( ) override;

	private:
		struct storage_type;
		std::unique_ptr<storage_type> storage_;
		//nstd::unordered_set<detail::players_filter> filter_cache__;
	};
}
