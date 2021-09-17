#pragma once

#include "selectable base.h"
#include "text.h"

#include <memory>

// ReSharper disable CppInconsistentNaming
struct ImGuiStyle;
struct ImVec4;
using ImU32 = unsigned int;
enum ImGuiCol_;
// ReSharper restore CppInconsistentNaming

namespace std
{
	template <typename T>
	class function;
	template <typename T>
	class optional;
}

namespace cheat::gui::tools
{
	class animator;
}

namespace cheat::gui::widgets
{
	class selectable_bg_colors_base;
	class selectable_bg;

	namespace detail
	{
		void add_default_selectable_callbacks(selectable_bg* owner);
	}

	//-------

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

		class colors_updater
		{
		public:
			colors_updater( );
			~colors_updater( );

			colors_updater(colors_updater&&) noexcept;
			colors_updater& operator=(colors_updater&&) noexcept;

			using usnset_clr = std::optional<ImGuiCol_>;

			void set_style_indexes(usnset_clr&& def, usnset_clr&& selected, usnset_clr&& hovered, usnset_clr&& held);
			void set_style_idx(color_priority priority, usnset_clr&& col);
			void change_color(color_priority priority, const ImVec4& color);
			void update_style(ImGuiStyle* style);

			const ImVec4& operator[](color_priority idx) const;

		private:
			struct impl;
			std::unique_ptr<impl> impl_;
		};

		//-----

		virtual colors_updater& get_colors_updater( ) =0;

		virtual void update_colors( ) =0;

		virtual void  change_color(color_priority clr) =0;
		virtual ImU32 calculate_color( ) =0;

		virtual color_priority get_last_color( ) const =0;
		virtual color_priority get_current_color( ) const =0;

		//todo: rect.min/max change
	};

	class selectable_bg_colors_fade: public selectable_bg_colors_base
	{
	public:
		selectable_bg_colors_fade( );
		~selectable_bg_colors_fade( ) override;

		selectable_bg_colors_fade(selectable_bg_colors_fade&&) noexcept;
		selectable_bg_colors_fade& operator=(selectable_bg_colors_fade&&) noexcept;

		colors_updater& get_colors_updater( ) override;

		void  update_colors( ) override;
		void  change_color(color_priority clr) override;
		ImU32 calculate_color( ) override;

		color_priority get_current_color( ) const override;
		color_priority get_last_color( ) const override;

		tools::animator&       fade( );
		const tools::animator& fade( ) const;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class selectable_bg_colors_static: public selectable_bg_colors_base
	{
	public:
		selectable_bg_colors_static( );
		~selectable_bg_colors_static( ) override;

		selectable_bg_colors_static(selectable_bg_colors_static&&) noexcept;
		selectable_bg_colors_static& operator=(selectable_bg_colors_static&&) noexcept;

		colors_updater& get_colors_updater( ) override;

		void  update_colors( ) override;
		void  change_color(color_priority clr) override;
		ImU32 calculate_color( ) override;

		color_priority get_current_color( ) const override;
		color_priority get_last_color( ) const override;

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
		bool         render(ImGuiWindow* window, ImRect& bb, ImGuiID id, bool outer_spacing);
		virtual void render_background(ImGuiWindow* window, ImRect& bb, selectable_bg_colors_base* color);

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
