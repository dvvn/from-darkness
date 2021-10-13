#pragma once

#include "shared_holder.h"

#include "cheat/sdk/QAngle.hpp"
#include "cheat/sdk/Vector.hpp"
#include "cheat/sdk/VMatrix.hpp"

#include <optional>
#include <vector>

namespace cheat
{
	namespace csgo
	{
		class C_BaseEntity;
	}

	class player;

	struct tick_record
	{
		csgo::Vector origin, abs_origin;
		csgo::QAngle rotation, abs_rotation;
		csgo::Vector mins, maxs;
		float sim_time;
		csgo::matrix3x4_t coordinate_frame;
		std::vector<csgo::matrix3x4_t> bones;

		void store_bones(csgo::C_BaseEntity* ent, std::optional<float> setup_curtime);
		bool is_valid(float curtime, float correct) const;

		tick_record() = default;
		tick_record(const player& holder);
	};

	using tick_record_shared = detail::shared_holder<tick_record>;
}
