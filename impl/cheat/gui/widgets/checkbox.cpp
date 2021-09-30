#include "checkbox.h"

#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/button_info.h"
#include "cheat/gui/tools/imgui_id.h"
#include "cheat/gui/tools/cached_text.h"

#include <imgui_internal.h>

#include <functional>

using nstd::smooth_value_base;
using namespace cheat::gui;
using namespace widgets;
using namespace tools;

button_state widgets::checkbox2(const cached_text& label,/* nstd::confirmable_value<bool>*/bool& value
							  , smooth_value_base<ImVec4>* bg_animation, smooth_value_base<ImVec4>* check_animation)
{
	const auto window = ImGui::GetCurrentWindow( );
	const auto& style = ImGui::GetStyle( );

	const auto square_sz = label.get_label_size(  ).y + style.FramePadding.y * 2.0f;
	const auto check_pos = window->DC.CursorPos;

	const auto check_bb  = ImRect(check_pos, check_pos + ImVec2(square_sz, square_sz));
	const auto label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y +label.get_label_size(  ).y*0.25f/*+ style.FramePadding.y*/);
	const auto label_bb  = ImRect(label_pos, label_pos + label.get_label_size( ));
	const auto total_bb  = ImRect(check_bb.Min, {label_bb.Max.x, check_bb.Max.y});

	const auto render_check = check_bb.Overlaps(window->ClipRect);
	const auto render_label = label_bb.Overlaps(window->ClipRect);

	ImGui::ItemSize(total_bb);
	if (!render_check && !render_label)
		return button_state::UNKNOWN;

	const auto id    = imgui_id(label, window);
	const auto state = button_behavior(check_bb, id, render_check == false);
	if (state == button_state::PRESSED)
		value = !value;

	/*if (!value.confirmed( ))
		value.confirm( );*/

	const auto& colors = style.Colors;
	if (render_check)
	{
		const auto bg_color = get_button_color(state, true
											 , colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBgHovered], colors[ImGuiCol_FrameBgActive], colors[ImGuiCol_FrameBgActive]
											 , bg_animation);
		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, bg_color);

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto check_color = get_boolean_color(value, colors[ImGuiCol_CheckMark], check_animation);
		if ((check_color & IM_COL32_A_MASK) > 0)
		{
			const auto pad = std::max(1.0f, IM_FLOOR(square_sz / 6.0f));
			ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_color, square_sz - pad * 2.0f);
		}
	}

	if (render_label)
		label.render(window->DrawList, label_pos, ImGui::GetColorU32(ImGuiCol_Text));

	return (state);
}
