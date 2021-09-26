#include "checkbox.h"

#include <imgui_internal.h>

#include <functional>

#include "cheat/gui/tools/button_info.h"

using namespace cheat::gui::widgets;
using namespace cheat::gui::tools;

struct checkbox::impl
{
	state current_state = STATE_IDLE;
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
		current_state = s;
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
	const auto window = ImGui::GetCurrentWindow( );
	const auto& style = ImGui::GetStyle( );

	const float square_sz = this->get_font( )->FontSize + style.FramePadding.y * 2.0f;
	const auto pos        = window->DC.CursorPos;

	auto check_bb        = ImRect(pos, pos + ImVec2(square_sz, square_sz));
	const auto label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
	const auto total_bb  = ImRect(pos, label_pos + this->get_label_size( ));

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
	auto target = std::make_unique<animation_property_target_external<ImVec4>>( );
	target->set_target(impl_->current_color);
	mod->set_target(std::move(target));
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

//---------

class animate_color_helper
{
	animation_property<ImVec4>* animation_;

public:
	explicit animate_color_helper(animation_property<ImVec4>* const animation)
		: animation_(animation)
	{
	}

	ImU32 operator()(const ImVec4& clr, bool update_end) const
	{
		ImVec4 result;
		if (!animation_)
		{
			result = clr;
		}
		else
		{
			if (update_end || animation_->get_end_val( ) != clr)
				animation_->update_end(clr);
			animation_->update( );
			result = animation_->get_target_value( );
		}
		const auto alpha = ImGui::GetStyle( ).Alpha;
		result.w *= alpha;
		return ImGui::ColorConvertFloat4ToU32(result);
	}
};

ImU32 get_selectable_color(button_state state, bool force_update_end
						 , const ImVec4& idle_clr, const ImVec4& hovered_clr, const ImVec4& held_clr, const ImVec4& pressed_clr
						 , animation_property<ImVec4>* animation)
{
	const auto get_color = animate_color_helper(animation);

	switch (state)
	{
		case button_state::IDLE:
			return get_color(idle_clr, force_update_end);
		case button_state::INACTIVE:
			return get_color(idle_clr, true);
		case button_state::HOVERED:
			return get_color(hovered_clr, true);
		case button_state::HOVERED_ACTIVE:
			return get_color(hovered_clr, force_update_end);
		case button_state::HELD:
			return get_color(held_clr, true);
		case button_state::HELD_ACTIVE:
			return get_color(held_clr, force_update_end);
		case button_state::PRESSED:
			return get_color(pressed_clr, true);
		default: throw;
	}
}

ImU32 get_check_color(bool changed
					, const ImVec4& clr
					, animation_property<ImVec4>* animation)
{
	const auto get_color = animate_color_helper(animation);
	return get_color(clr, changed);
}

button_state cheat::gui::widgets::checkbox2(const cached_text& label, confirmable_value<bool>& value, animation_property<ImVec4>* bg_animation
										  , animation_property<ImVec4>* check_animation)
{
	const auto window = ImGui::GetCurrentWindow( );
	const auto& style = ImGui::GetStyle( );

	const float square_sz = label.get_font( )->FontSize + style.FramePadding.y * 2.0f;
	const auto pos        = window->DC.CursorPos;

	const auto check_bb  = ImRect(pos, pos + ImVec2(square_sz, square_sz));
	const auto label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
	const auto text_bb   = ImRect(label_pos, label_pos + label.get_label_size( ));
	//const auto total_bb  = ImRect(pos, label_pos + label.get_label_size( ));

	const auto render_check = check_bb.Overlaps(window->ClipRect);
	const auto render_text  = text_bb.Overlaps(window->ClipRect);

	ImGui::ItemSize({check_bb.Min, text_bb.Max});
	if (!render_check && !render_text)
		return button_state::UNKNOWN;
	/*ImGui::ItemSize(total_bb.GetSize( ));
	if (!total_bb.Overlaps(window->ClipRect))
		return button_state::UNKNOWN;*/

	const auto id = make_imgui_id(label, window);

	auto value_changed = false;
	const auto state   = button_behavior(check_bb, id);
	if (state == button_state::PRESSED)
	{
		value = !value;
		value.confirm( );
		value_changed = true;
	}
	else
	{
		//changed externally
		if (!value.confirmed( ))
		{
			value_changed = true;
			value.confirm( );
		}
	}

	const auto& colors = style.Colors;
	if (render_check)
	{
		const auto bg_color = [&]
		{
			/*auto idle_color = colors[ImGuiCol_FrameBg];//for selectable not checkbox
			if (!value)
				idle_color.w = 0;*/

			return get_selectable_color(state, value_changed
									  , /*idle_color*/colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBgHovered], colors[ImGuiCol_FrameBgActive], colors[ImGuiCol_FrameBgActive]
									  , bg_animation);
		};
		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, bg_color( ));

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto check_color = [&]
		{
			auto color = colors[ImGuiCol_CheckMark];
			if (!value)
				color.w = 0;

			return get_check_color(value_changed, color, check_animation);
		}( );
		if ((check_color & IM_COL32_A_MASK) > 0)
		{
			const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
			ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_color, square_sz - pad * 2.0f);
		}
	}

	if (render_text)
		label.render(window->DrawList, label_pos, ImGui::GetColorU32(ImGuiCol_Text));

	return (state);
}
