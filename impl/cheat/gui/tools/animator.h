#pragma once

using int8_t = signed char;

namespace cheat::gui::tools
{
	class animator
	{
	public:
		using float_type=float;

		static constexpr float_type default_min  = 0;
		static constexpr float_type default_max  = 1;
		static constexpr float_type default_time = 0.25;

		int8_t dir( ) const;

		bool updating( ) const;
		void set(int8_t direction);
		bool update( );
		void finish( );

		void restart( );

		bool  done( ) const;
		bool  done(int8_t direction) const;
		float_type value( ) const;

		animator(float_type value_min = default_min, float_type value_max = default_max, float_type time_max = default_time);

		void set_min(float_type val);
		void set_max(float_type val);
		void set_min_max(float_type min, float_type max);
		void set_time(float_type val);

		float_type min( ) const;
		float_type max( ) const;
		float_type time( ) const;

	private:
		float_type get_limit(int8_t dir) const;

		int8_t dir_  = 0;
		float_type  time_ = 0;

		struct
		{
			float_type min;
			float_type max;
			float_type current = 0;
			//-
		} value_;

		float_type time_max_;
	};
}
