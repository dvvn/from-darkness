#include "checkbox.h"

#include "cheat/gui/tools/string wrapper.h"

#include <imgui_internal.h>

using namespace cheat::gui::widgets;

struct checkbox::impl
{
	std::unique_ptr<selectable_bg_colors_base> check_colors;
};

checkbox::checkbox( )
{
	impl_ = std::make_unique<impl>( );
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

	const auto id      = this->get_id(window);
	auto       cb_data = callback_data_ex(tools::callback_data(this, id));

	this->invoke_button_callbacks(check_bb, cb_data);
	this->render_background(window, check_bb, *this->get_bg_colors( ));

	const auto check_color = this->get_check_colors( )->calculate_color( );
	if ((check_color & IM_COL32_A_MASK) != 0)
	{
		const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
		ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_color, square_sz - pad * 2.0f);
	}

	this->render_text(window, label_pos);

	/*const auto& label_size = this->label_size( );

	const float square_sz = ImGui::GetFrameHeight( );
	const auto  pos       = window->DC.CursorPos;
	auto        bb        = ImRect(pos, pos + ImVec2(square_sz + (style.ItemInnerSpacing.x + label_size.x), label_size.y + style.FramePadding.y * 2.0f));
	ImGui::ItemSize(bb, style.FramePadding.y);

	const auto id      = this->get_id(window);
	auto       cb_data = callback_data_ex(tools::callback_data(this, id));

	if (!selectable_bg::render(window, bb, cb_data, false))
		return;

	*/

	/*ImGui::Checkbox( )

	

	const float square_sz = ImGui::GetFrameHeight( );
	auto        bb        = this->make_rect(window);

	if (square_sz > bb.Max.x)
		bb.Max.x = square_sz;

	ImGui::ItemSize(bb.GetSize( ));
	const auto check_pos = bb.Min;

	const auto id      = this->get_id(window);
	auto       cb_data = callback_data_ex(tools::callback_data(this, id));

	if (!selectable_bg::render(window, bb, cb_data, false))
		return;

	const auto check_color = impl_->check_colors->calculate_color( );

	if ((check_color & IM_COL32_A_MASK) != 0)
	{
		const ImRect check_bb(check_pos, check_pos + ImVec2(square_sz, square_sz));
	}
	const auto text_pos = text_pos +
						  this->render_text(window, text_pos);*/
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

//ImGuiButtonFlags_ checkbox::get_button_flags( ) const
//{
//	std::underlying_type_t<ImGuiButtonFlags_> flags = selectable_bg::get_button_flags( );
//	if (!this->selected( ))
//		flags |= ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_NoHoldingActiveId;
//	return static_cast<ImGuiButtonFlags_>(flags);
//}
