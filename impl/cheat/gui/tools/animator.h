#pragma once

#define CHEAT_GUI_WIDGETS_FADE_CONTENT

namespace cheat::gui::tools
{
	class animator
	{
	public:
		static constexpr float default_min = 0;
		static constexpr float default_max = 1;
		static constexpr float default_time = 0.3f;

		int dir( ) const;

		bool updating( ) const;
		void set(int direction);
		bool update( );
		void finish( );

		bool done( ) const;
		bool done(int direction) const;
		float value( ) const;

		

		animator(float value_min = default_min, float value_max = default_max, float time_max = default_time);

		void set_min(float val);
		void set_max(float val);
		void set_min_max(float min, float max);
		void set_time(float val);

		float min( ) const;
		float max( ) const;
		float time( ) const;

	private:
		float Limit_(float dir) const;

		int dir__ = 0;
		float time__ = 0;
		struct 
		{
			float min;
			float max;
			float current=0;
		} value__;
		float time_max__;
	};

	class widget_animator
	{
	protected:
		widget_animator(animator&& a = { }): fade_(utl::move(a))
		{
		}

	public:
		bool animating( ) const;

	protected:
		bool Animate( );

		animator fade_;
		utl::memory_backup<float> fade_alpha_backup_;
	};
}
