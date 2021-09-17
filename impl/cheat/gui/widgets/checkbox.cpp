#include "checkbox.h"

#include "cheat/gui/tools/string wrapper.h"

#include <imgui_internal.h>

#include <functional>

using namespace cheat::gui::widgets;

void detail::add_default_checkbox_callbacks(checkbox* owner)
{
	using namespace cheat::gui::tools;

	owner->add_pressed_callback(make_callback_info([=]
	{
		owner->toggle( );
	}), two_way_callback::WAY_TRUE);
	owner->add_selected_callback(make_callback_info([=]
	{
		const auto colors = owner->get_check_colors( );
		colors->change_color(selectable_bg_colors_base::COLOR_SELECTED);
	}), two_way_callback::WAY_TRUE);
	owner->add_selected_callback(make_callback_info([=]
	{
		const auto colors = owner->get_check_colors( );
		colors->change_color(selectable_bg_colors_base::COLOR_DEFAULT);
	}), two_way_callback::WAY_FALSE);
}

//----

struct checkbox::impl
{
	std::unique_ptr<selectable_bg_colors_base> check_colors;
};

checkbox::checkbox( )
{
	impl_ = std::make_unique<impl>( );
	detail::add_default_checkbox_callbacks(this);
}

checkbox::~checkbox( )                             = default;
checkbox::checkbox(checkbox&&) noexcept            = default;
checkbox& checkbox::operator=(checkbox&&) noexcept = default;

void checkbox::render( )
{
	const auto  window = ImGui::GetCurrentWindow( );
	const auto& style  = ImGui::GetStyle( );

	const float square_sz = this->get_font( )->FontSize + style.FramePadding.y * 2.0f;
	const auto  pos       = window->DC.CursorPos;

	auto       check_bb  = ImRect(pos, pos + ImVec2(square_sz, square_sz));
	const auto label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
	const auto total_bb  = ImRect(pos, label_pos + this->label_size( ));

	ImGui::ItemSize(total_bb.GetSize( ));
	if (!total_bb.Overlaps(window->ClipRect))
		return;

	const auto id = this->get_id(window);

	this->invoke_button_callbacks(check_bb, id);
	this->render_background(window, check_bb, this->get_bg_colors( ));
	this->render_check_mark(window, check_bb.Min, square_sz, this->get_check_colors( ));

	this->render_text(window, label_pos);
}

void checkbox::set_check_colors(std::unique_ptr<selectable_bg_colors_base>&& colors)
{
	colors->update_colors( );
	impl_->check_colors = std::move(colors);
}

selectable_bg_colors_base* checkbox::get_check_colors( )
{
	return impl_->check_colors.get( );
}

void checkbox::render_check_mark(ImGuiWindow* window, const ImVec2& basic_pos, float basic_size, selectable_bg_colors_base* colors)
{
	const auto check_color = colors->calculate_color( );
	if ((check_color & IM_COL32_A_MASK) != 0)
	{
		const float pad = ImMax(1.0f, IM_FLOOR(basic_size / 6.0f));
		ImGui::RenderCheckMark(window->DrawList, basic_pos + ImVec2(pad, pad), check_color, basic_size - pad * 2.0f);
	}
}

//ImGuiButtonFlags_ checkbox::get_button_flags( ) const
//{
//	std::underlying_type_t<ImGuiButtonFlags_> flags = selectable_bg::get_button_flags( );
//	if (!this->selected( ))
//		flags |= ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_NoHoldingActiveId;
//	return static_cast<ImGuiButtonFlags_>(flags);
//}
