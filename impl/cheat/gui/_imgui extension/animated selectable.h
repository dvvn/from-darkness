#pragma once

#include "cheat/gui/animator.h"
#include "string wrapper.h"

namespace cheat::gui::imgui
{
	class animated_selectable
	{
	public:
		animated_selectable(bool selected = false);

		bool operator()(string_wrapper::value_type label, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));

		void select( );
		void deselect( );
		void toggle( );

		bool selected( ) const;
		bool animating( ) const;
	private:
		animator anim__;
	};

	class animated_selectable_base: public animated_selectable
	{
		using animated_selectable::operator();

	public:
		virtual ~animated_selectable_base( ) = default;

		animated_selectable_base(bool selected = false);

		bool operator()(ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));

	protected:
		virtual string_wrapper::value_type Name( ) const =0;
	};
}
