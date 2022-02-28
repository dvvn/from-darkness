module;

#include <vector>
#include <optional>

export module cheat.players:tick_record;
export import :shared_holder;
export import cheat.csgo.interfaces.C_BaseAnimating;

export namespace cheat
{
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

	using tick_record_shared = shared_holder<tick_record>;
}
