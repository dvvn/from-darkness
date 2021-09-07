#pragma once
#include "abstract page.h"

#include "cheat/gui/widgets/group.h"
#include "cheat/gui/widgets/window.h"

#define CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING

namespace cheat::gui::objects
{
#if 0

	[[deprecated]]
	class vertical_pages_renderer final: public abstract_pages_renderer, widgets::child_frame_window
	{
	public:
		void render( ) override;
		void init( ) override;

		using child_frame_window::show;

	private:
		size_t         longest_string__ = 0;
		widgets::group selected_group__;
	};

	[[deprecated]]
	class horizontal_pages_renderer final: public abstract_pages_renderer, widgets::child_frame_window
	{
	public:
		horizontal_pages_renderer(/*size_t per_line_limit=-1*/) = default;

		void render( ) override;
		void init( ) override;

		using child_frame_window::show;

	private:
		size_t
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
		longest_string__
#else
			chars_count__
#endif
			= 0;
		widgets::group selected_group__;
		//size_t per_line_limit__; todo
	};

#endif

	//---------------------

	//unused
	class shared_child_windows_storage
	{
	public:
		using shared_window = std::shared_ptr<widgets::child_window>;

		shared_child_windows_storage( );
		~shared_child_windows_storage( );

		size_t size( ) const;
		bool   empty( ) const;

		void add(const shared_window& window);

		const shared_window& operator[](size_t i) const;

		const shared_window* begin( ) const;
		const shared_window* end( ) const;

	private:
		struct storage_type;
		std::unique_ptr<storage_type> storage_;
	};

	class abstract_label
	{
	public:
		virtual ~abstract_label( ) = default;

		virtual const tools::string_wrapper& label( ) const = 0;
	};

	//move outside
	class non_abstract_label: public abstract_label
	{
	public:
		non_abstract_label(tools::string_wrapper&& label);
		~non_abstract_label( ) override;

		const tools::string_wrapper& label( ) const final;

	private:
		tools::string_wrapper label_;
	};

	class tab_bar final: public renderable
	{
	public:
		tab_bar( );
		~tab_bar( ) override;

		using shared_label = std::shared_ptr<abstract_label>;

		void   add(const shared_label& label);
		size_t selected( ) const;

		void calc_size_static( );
		void call_size_auto( );

		void make_horisontal( );
		void make_vertical( );

		bool is_horisontal( ) const;
		bool is_vertical( ) const;

		void render( ) override;

	private:
		class item;
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
