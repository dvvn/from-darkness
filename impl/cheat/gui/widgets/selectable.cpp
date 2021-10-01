#include "selectable.h"

#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/button_info.h"
#include "cheat/gui/tools/cached_text.h"
#include "cheat/gui/tools/imgui_id.h"

#include <imgui_internal.h>

#include <cmath>

using nstd::smooth_value_base;

using namespace cheat::gui;
using namespace widgets;
using namespace tools;

button_state widgets::selectable2(const cached_text& label, bool selected, smooth_value_base<ImVec4>* bg_animation)
{
	const auto window = ImGui::GetCurrentWindow( );
	const auto& style = ImGui::GetStyle( );

	const auto label_pos = [&]
	{
		// ReSharper disable once CppUseStructuredBinding
		const auto& dc = window->DC;
		return ImVec2(dc.CursorPos.x, dc.CursorPos.y + dc.CurrLineTextBaseOffset);;
	}( );
	auto bb = ImRect(label_pos, label_pos + label.label_size);
	ImGui::ItemSize(bb.GetSize( ));

	// ReSharper disable once CppIfCanBeReplacedByConstexprIf
	if (/*outer_spacing*/true)
	{
		const auto& spacing = style.ItemSpacing;
		const auto spacing2 = ImVec2(std::floor(spacing.x / 2.f), std::floor(spacing.y / 2.f));

		bb.Min -= spacing2;
		bb.Max += spacing - spacing2;

#if 0
		const auto spacing_x = /*span_all_columns ? 0.0f :*/ style.ItemSpacing.x;
		const auto spacing_y = style.ItemSpacing.y;
		const auto spacing_L = std::floor(spacing_x * 0.50f);
		const auto spacing_U = std::floor(spacing_y * 0.50f);
		bb.Min.x -= spacing_L;
		bb.Min.y -= spacing_U;
		bb.Max.x += spacing_x - spacing_L;
		bb.Max.y += spacing_y - spacing_U;
#endif
	}

	if (!bb.Overlaps(window->ClipRect))
		return button_state::UNKNOWN;

	const auto id    = imgui_id(label, window);
	const auto state = button_behavior(bb, id);

	const auto bg_color = [&]
	{
		const auto& colors = style.Colors;
		return get_button_color(state, selected,
								colors[ImGuiCol_Header], colors[ImGuiCol_HeaderHovered], colors[ImGuiCol_HeaderActive]
							  , bg_animation);
	};
	window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color( ));

	label.render(window->DrawList, label_pos, ImGui::GetColorU32(ImGuiCol_Text));
	return state;
}
