#pragma once

#include "shared holder.h"

namespace cheat
{
	struct player;
	struct tick_record: detail::shared_holder_info
	{
		utl::Vector origin, abs_origin;
		utl::QAngle rotation, abs_rotation;
		utl::Vector mins, maxs;
		float sim_time;
		utl::matrix3x4_t coordinate_frame;

		bool is_valid(float curtime) const;
	};

	using tick_record_shared = utl::shared_ptr<tick_record>;

	namespace detail
	{
		class tick_record_shared_impl: public shared_holder<tick_record>
		{
		public:
			void init(const player& holder);

		
		};
	}
}
