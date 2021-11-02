#pragma once

#include "shared_holder.h"

#include "cheat/csgo/QAngle.hpp"
#include "cheat/csgo/Vector.hpp"
#include "cheat/csgo/VMatrix.hpp"

#include <vector>

namespace std
{
	template <class _Ty>
	class optional;
}

namespace cheat
{
	namespace csgo
	{
		class C_BaseAnimating;
		class C_BaseEntity;
		struct CAnimationLayer;
	}

	class player;

	struct tick_record
	{
		tick_record( ) = default;

		tick_record(const player& holder);
		csgo::Vector origin, abs_origin;
		csgo::QAngle rotation, abs_rotation;
		csgo::Vector mins, maxs;
		float sim_time;
		csgo::matrix3x4_t coordinate_frame;

		void store_bones(csgo::C_BaseEntity* ent, const std::optional<float>& setup_curtime);
		std::vector<csgo::matrix3x4_t> bones;

		void store_animations(csgo::C_BaseAnimating* ent);
		std::vector<csgo::CAnimationLayer> layers;
		std::vector<float> poses;

		bool is_valid(float curtime, float correct) const;
	};

	using tick_record_shared = detail::shared_holder<tick_record>;
}
