#pragma once

#include "shared holder.h"

#include "cheat/sdk/QAngle.hpp"
#include "cheat/sdk/Vector.hpp"
#include "cheat/sdk/VMatrix.hpp"

#include <span>

namespace cheat
{
	namespace csgo
	{
		class C_BaseEntity;
	}

	struct player;

	namespace detail
	{
		class stored_player_bones: public std::span<csgo::matrix3x4_t>
		{
		public:
			stored_player_bones(csgo::C_BaseEntity*ent);
			stored_player_bones( ) = default;

		private:
			std::unique_ptr<csgo::matrix3x4_t[]> cache__;
		};
	}

	struct tick_record: detail::shared_holder_info
	{
		csgo::Vector origin, abs_origin;
		csgo::QAngle rotation, abs_rotation;
		csgo::Vector mins, maxs;
		float sim_time;
		csgo::matrix3x4_t coordinate_frame;

		detail::stored_player_bones bones;

		bool is_valid(float curtime) const;
	};

	using tick_record_shared = std::shared_ptr<tick_record>;

	namespace detail
	{
		class tick_record_shared_impl final: public shared_holder<tick_record>
		{
		public:
			void init(const player& holder);
			void store_bones(csgo::C_BaseEntity*ent );
		};
	}
}
