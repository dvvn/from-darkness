#include "selectable.h"

#include "cheat/gui/tools/push style color.h"

#include <imgui_internal.h>

#include <algorithm>
#include <vector>
#include <functional>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable::data_type
{
	std::vector<callback_type> on_pressed;
};

selectable::selectable( )
{
	data_ = std::make_unique<data_type>( );
}

selectable::~selectable( )                               = default;
selectable::selectable(selectable&&) noexcept            = default;
selectable& selectable::operator=(selectable&&) noexcept = default;

void selectable::render( )
{
	const auto  window = ImGui::GetCurrentWindow( );
	const auto& style  = ImGui::GetStyle( );

	/*if (window->SkipItems)// uselles, window->begin already check it
		return;*/

	auto bb = this->make_rect(window);
	ImGui::ItemSize(bb.GetSize( ));
	const auto text_pos = bb.Min;

	const float spacing_x = /*span_all_columns ? 0.0f :*/ style.ItemSpacing.x;
	const float spacing_y = style.ItemSpacing.y;
	const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
	const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
	bb.Min.x -= spacing_L;
	bb.Min.y -= spacing_U;
	bb.Max.x += (spacing_x - spacing_L);
	bb.Max.y += (spacing_y - spacing_U);

	const auto id = this->get_id(window);

	/*if (!ImGui::ItemAdd(bb, id))
		return;*/
	if (!bb.Overlaps(window->ClipRect))
		return;

	bool       hovered, held;
	const auto pressed   = ImGui::ButtonBehavior(bb, id, &hovered, &held);
	const auto animating = this->Animate( );

	if (pressed)
	{
		/*if (pressed)
			ImGui::MarkItemEdited(id);*/
		for (const auto& fn: data_->on_pressed)
			fn(this);
	}

	const auto render_color = [&]( )-> std::optional<ImU32>
	{
		if (hovered)
		{
			if (held)
				return ImGui::GetColorU32(ImGuiCol_HeaderActive);
			return ImGui::GetColorU32(ImGuiCol_HeaderHovered);
		}
		if (animating)
		{
			auto clr = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			clr.w *= this->Anim_value( );
			return ImGui::ColorConvertFloat4ToU32(clr);
		}
		if (pressed || this->selected( ))
			return ImGui::GetColorU32(ImGuiCol_Header);

		return { };
	}( );

	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	if (render_color.has_value( ))
		window->DrawList->AddRectFilled(bb.Min, bb.Max, *render_color);

	this->render_text(window, text_pos);

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

	//text::render( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void selectable::add_pressed_callback(callback_type&& callback)
{
	data_->on_pressed.push_back(std::move(callback));
}
