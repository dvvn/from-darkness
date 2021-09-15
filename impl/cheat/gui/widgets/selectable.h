#pragma once

#include "selectable base.h"
#include "text.h"

#include <memory>

// ReSharper disable CppInconsistentNaming
using ImU32 = unsigned int;
// ReSharper restore CppInconsistentNaming

namespace std
{
	template <typename T>
	class function;
}

namespace cheat::gui::widgets
{
	class selectable_bg;

	class selectable_bg_colors_base
	{
	public:
		virtual ~selectable_bg_colors_base( ) = default;

		enum color_priority :uint8_t
		{
			COLOR_DEFAULT
		  , COLOR_SELECTED
		  , COLOR_HOVERED
		  , COLOR_HELD
		};

		virtual void  update_colors(selectable_bg* owner) =0;
		virtual void  set_color(color_priority clr) =0;
		virtual ImU32 get_color( ) =0;

		//todo: rect.min/max change

	private:
	};

	class selectable_bg_colors_fade final: public selectable_bg_colors_base
	{
	public:
		selectable_bg_colors_fade( );
		~selectable_bg_colors_fade( ) override;

		selectable_bg_colors_fade(selectable_bg_colors_fade&&) noexcept;
		selectable_bg_colors_fade& operator=(selectable_bg_colors_fade&&) noexcept;

		void  update_colors(selectable_bg* owner) override;
		void  set_color(color_priority clr) override;
		ImU32 get_color( ) override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class selectable_bg_colors_static final: public selectable_bg_colors_base
	{
	public:
		selectable_bg_colors_static( );
		~selectable_bg_colors_static( ) override;

		selectable_bg_colors_static(selectable_bg_colors_static&&) noexcept;
		selectable_bg_colors_static& operator=(selectable_bg_colors_static&&) noexcept;

		void  update_colors(selectable_bg* owner) override;
		void  set_color(color_priority clr) override;
		ImU32 get_color( ) override;

	protected:
	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class selectable_bg: public selectable_base
	{
	public:
		selectable_bg( );
		~selectable_bg( ) override;

		selectable_bg(selectable_bg&&) noexcept;
		selectable_bg& operator=(selectable_bg&&) noexcept;

		void                       set_bg_colors(std::unique_ptr<selectable_bg_colors_base>&& colors);
		selectable_bg_colors_base* get_bg_colors( );

	protected:
		bool render(ImGuiWindow* window, ImRect& bb, callback_data_ex& cb_data, bool outer_spacing);

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	class selectable: public selectable_bg
					, public text
	{
	public:
		selectable( );
		~selectable( ) override;

		selectable(selectable&&) noexcept;
		selectable& operator=(selectable&&) noexcept;

		void render( ) override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};
}
