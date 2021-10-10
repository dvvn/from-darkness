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

	class player;

	namespace detail
	{
		using bones_cache_copy_base = std::span<csgo::matrix3x4_t>;

		struct bones_cache_copy : bones_cache_copy_base
		{
			using base = bones_cache_copy_base;

			bones_cache_copy(const bones_cache_copy& other)            = delete;
			bones_cache_copy& operator=(const bones_cache_copy& other) = delete;

			bones_cache_copy(bones_cache_copy&& other) noexcept;
			bones_cache_copy& operator=(bones_cache_copy&& other) noexcept;

			bones_cache_copy() = default;
			~bones_cache_copy();
		};
	}

	struct tick_record
	{
		csgo::Vector origin, abs_origin;
		csgo::QAngle rotation, abs_rotation;
		csgo::Vector mins, maxs;
		float sim_time;
		csgo::matrix3x4_t coordinate_frame;
		detail::bones_cache_copy bones;

		void store_bones(csgo::C_BaseEntity* ent);
		bool is_valid(float curtime) const;

		tick_record() = default;
		tick_record(const player& holder);
	};

	using tick_record_shared = detail::shared_holder<tick_record>;
}
