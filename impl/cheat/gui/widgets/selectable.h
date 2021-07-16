#pragma once

#include "cheat/gui/tools/animator.h"
#include "cheat/gui/tools/string wrapper.h"

namespace cheat::gui::widgets
{
	class selectable
	{
	public:
		selectable(bool selected = false);

		bool operator()(tools::string_wrapper::value_type label, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
		bool operator()(const tools::string_wrapper& label, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));

		void select( );
		void deselect( );
		void toggle( );

		bool selected( ) const;
		bool animating( ) const;

	private:
		tools::animator anim__;
	};

	class selectable_base: public selectable
	{
		using selectable::operator();

	public:
		virtual ~selectable_base( ) = default;

		selectable_base(bool selected = false);

		bool operator()(ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));

	protected:
		virtual tools::string_wrapper::value_type Name( ) const =0;
	};
}
