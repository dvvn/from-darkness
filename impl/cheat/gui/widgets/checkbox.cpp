#include "checkbox.h"

#include "cheat/gui/tools/string wrapper.h"

#include <imgui_internal.h>

#include <functional>

using namespace cheat::gui::widgets;

struct checkbox::impl
{
	state  current_state = STATE_IDLE;
	ImVec4 current_color;

	struct
	{
		ImVec4 def, selected;

		ImVec4& operator[](state s)
		{
			switch (s)
			{
				case STATE_IDLE: return def;
				case STATE_SELECTED: return selected;
				default: throw;
			}
		}
	} colors;

	std::unique_ptr<animation_property<ImVec4>> color_modifier;

	void set_state(state s)
	{
		const auto& new_color = colors[s];

		if (color_modifier == nullptr)
		{
			current_color = new_color;
		}
		else
		{
			const auto& prev_color = colors[current_state];
			color_modifier->set_start(prev_color);
			color_modifier->set_end(new_color);
			color_modifier->start(true, true);
		}
		current_state=s;
	}

	impl()
	{
		//temp solution
		auto& sc = ImGui::GetStyle( ).Colors;

		colors.def      = /*sc[ImGuiCol_FrameBg]*/{};
		colors.selected = sc[ImGuiCol_CheckMark];
	}
};

checkbox::checkbox()
{
	using namespace tools;

	impl_ = std::make_unique<impl>( );
	this->add_pressed_callback(make_callback_info([=]
	{
		this->toggle( );
	}), two_way_callback::WAY_TRUE);
	this->add_selected_callback(make_callback_info([=]
	{
		impl_->set_state(STATE_SELECTED);
	}), two_way_callback::WAY_TRUE);
	this->add_selected_callback(make_callback_info([=]
	{
		impl_->set_state(STATE_IDLE);
	}), two_way_callback::WAY_FALSE);
}

checkbox::~checkbox()                              = default;
checkbox::checkbox(checkbox&&) noexcept            = default;
checkbox& checkbox::operator=(checkbox&&) noexcept = default;

void checkbox::render()
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
	this->render_background(window, check_bb);
	this->render_check_mark(window, check_bb.Min, square_sz);

	this->render_text(window, label_pos);
}

void checkbox::set_check_color_modifier(std::unique_ptr<animation_property<ImVec4>>&& mod)
{
	mod->set_target(impl_->current_color);
	impl_->color_modifier = std::move(mod);
}

void checkbox::render_check_mark(ImGuiWindow* window, const ImVec2& basic_pos, float basic_size)
{
	if (impl_->color_modifier)
		impl_->color_modifier->update( );

	const auto& check_color = impl_->current_color;
	if (check_color.w > 0)
		//if ((check_color & IM_COL32_A_MASK) != 0)
	{
		const float pad = ImMax(1.0f, IM_FLOOR(basic_size / 6.0f));
		ImGui::RenderCheckMark(window->DrawList, basic_pos + ImVec2(pad, pad), ImGui::ColorConvertFloat4ToU32(check_color), basic_size - pad * 2.0f);
	}
}

//ImGuiButtonFlags_ checkbox::get_button_flags( ) const
//{
//	std::underlying_type_t<ImGuiButtonFlags_> flags = selectable_bg::get_button_flags( );
//	if (!this->selected( ))
//		flags |= ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_NoHoldingActiveId;
//	return static_cast<ImGuiButtonFlags_>(flags);
//}
