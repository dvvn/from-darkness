#pragma once

#include "shared holder.h"

namespace cheat
{
	namespace csgo
	{
		class C_BaseEntity;
	}

	struct player;

	namespace detail
	{
		class stored_player_bones: public utl::span<utl::matrix3x4_t>
		{
		public:
			stored_player_bones(csgo::C_BaseEntity*ent);
			stored_player_bones( ) = default;

		private:
			utl::unique_ptr<utl::matrix3x4_t[]> cache__;
		};
	}

	struct tick_record: detail::shared_holder_info
	{
		utl::Vector origin, abs_origin;
		utl::QAngle rotation, abs_rotation;
		utl::Vector mins, maxs;
		float sim_time;
		utl::matrix3x4_t coordinate_frame;

		detail::stored_player_bones bones;

		bool is_valid(float curtime) const;
	};

	using tick_record_shared = utl::shared_ptr<tick_record>;

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
