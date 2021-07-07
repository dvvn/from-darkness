#pragma once

namespace cheat::gui
{
	class animator
	{
	public:
		auto dir( ) const -> int;

		auto updating( ) const -> bool;
		auto set(int direction) -> void;
		auto update( ) -> bool;
		auto finish( ) -> void;

		auto done( ) const -> bool;
		auto done(int direction) const -> bool;
		auto value( ) const -> float;

		//auto setup_limits(float value_min=0, float value_max=1, float time_max=0.3f) -> void;

		animator(float value_min = 0, float value_max = 1, float time_max = 0.3f);

	private:
		auto Limit_(float dir ) const -> float;

		float time_max__ = 0;
		int   dir__ = 0;
		float time__ = 0;
		struct
		{
			float min = 0;
			float max = 1;
			float current = 0;
			//-
		} value__;
	};
}
