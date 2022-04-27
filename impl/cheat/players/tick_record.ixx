module;

#include <vector>
#include <optional>

export module cheat.players.tick_record;
export import cheat.players.shared_holder;
export import cheat.csgo.interfaces.C_BaseAnimating;

export namespace cheat::players
{
	struct tick_record
	{
		//tick_record(const player& holder);
		math::vector3 origin, abs_origin;
		math::qangle rotation, abs_rotation;
		math::vector3 mins, maxs;
		float sim_time;
		math::matrix3x4 coordinate_frame;

		void store_bones(csgo::C_BaseEntity* ent, const std::optional<float>& setup_curtime);
		std::vector<math::matrix3x4> bones;

		void store_animations(csgo::C_BaseAnimating* ent);
		std::vector<csgo::CAnimationLayer> layers;
		std::vector<float> poses;

		bool is_valid(float curtime, float correct) const;
	};

	using tick_record_shared = shared_holder<tick_record>;
}
