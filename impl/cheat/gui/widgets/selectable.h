#pragma once

#include "selectable base.h"

#include "cheat/gui/tools/string wrapper.h"

#include "text.h"

#include <imgui.h>
#include <memory>

namespace std
{
	template <typename T>
	class function;
}

namespace cheat::gui::widgets
{
	class selectable: public selectable_base
	{
	public:
		selectable(bool selected = false);

		bool operator()(tools::perfect_string&& label,
						ImGuiSelectableFlags_   flags = ImGuiSelectableFlags_None, const ImVec2& size = ImVec2(0, 0));
	};

	class selectable2: public selectable_base
					 , public text
	{
	public:
		selectable2( );
		~selectable2( ) override;

		selectable2(selectable2&&) noexcept;
		selectable2& operator=(selectable2&&) noexcept;

		void render( ) override;
		//this->selected( ) ? this->deselect( ) : this->select( );
		using callback_type = std::function<void(selectable2*)>;

		void add_pressed_callback(callback_type&& callback);

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	/*class selectable_internal: public selectable
	{
		using selectable::operator();

	protected:
		selectable_internal(bool selected = false);

		virtual tools::string_wrapper::value_type Label( ) const = 0;

	public:
		bool operator()(ImGuiSelectableFlags_ flags = ImGuiSelectableFlags_None, const ImVec2& size = ImVec2(0, 0));
	};*/
}
