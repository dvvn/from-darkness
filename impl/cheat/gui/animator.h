#pragma once

namespace cheat::gui
{
	class animator
	{
	public:
		int dir( ) const;

		bool updating( ) const;
		void set(int direction);
		bool update( );
		void finish( );

		bool  done( ) const;
		bool  done(int direction) const;
		float value( ) const;

		//auto setup_limits(float value_min=0, float value_max=1, float time_max=0.3f) -> void;

		animator(float value_min = 0, float value_max = 1, float time_max = 0.3f);

	private:
		float Limit_(float dir) const;

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
