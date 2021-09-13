#pragma once

using int8_t = signed char;

namespace cheat::gui::tools
{
	class animator
	{
	public:
		static constexpr double default_min  = 0;
		static constexpr double default_max  = 1;
		static constexpr double default_time = 0.4;

		int8_t dir( ) const;

		bool updating( ) const;
		void set(int8_t direction);
		bool update( );
		void finish( );

		void restart( );

		bool   done( ) const;
		bool   done(int direction) const;
		double value( ) const;

		animator(double value_min = default_min, double value_max = default_max, double time_max = default_time);

		void set_min(double val);
		void set_max(double val);
		void set_min_max(double min, double max);
		void set_time(double val);

		double min( ) const;
		double max( ) const;
		double time( ) const;

	private:
		double get_limit(int8_t dir) const;

		int8_t dir_  = 0;
		double time_ = 0;

		struct
		{
			double min;
			double max;
			double current = 0;
			//-
		} value_;

		double time_max_;
	};
}
