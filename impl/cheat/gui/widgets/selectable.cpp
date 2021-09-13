#include "selectable.h"

#include "cheat/core/console.h"
#include "cheat/gui/tools/push style color.h"

#include <array>
#include <format>
#include <imgui_internal.h>

#include <functional>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable::data_type
{
	enum color_priority:uint8_t
	{
		COLOR_DEFAULT,
		COLOR_SELECTED,
		COLOR_HOVERED,
		COLOR_HELD
	};

	std::array<ImVec4, 4> colors;

	data_type( )
	{
		fade.set(-1);
		fade.finish( );

		auto& clr = ImGui::GetStyle( ).Colors;

		colors[COLOR_DEFAULT]  = { }; //no color
		colors[COLOR_SELECTED] = clr[ImGuiCol_Header];
		colors[COLOR_HOVERED]  = clr[ImGuiCol_HeaderHovered];
		colors[COLOR_HELD]     = clr[ImGuiCol_HeaderActive];

		clr_last = clr_curr = COLOR_DEFAULT;
	}

	tools::animator fade;

	color_priority clr_last, clr_curr;

	void set_color(color_priority clr)
	{
		clr_last = clr_curr;
		clr_curr = clr;

		if (clr_last == clr_curr)
			return;

		fade.set(clr_last < clr_curr ? 1 : -1);
	}

	ImU32 get_color( )
	{
		if (!fade.update( ))
			return ImGui::ColorConvertFloat4ToU32(colors[clr_curr]);

		const auto& from = colors[std::min(clr_last, clr_curr)];
		const auto& to   = colors[std::max(clr_last, clr_curr)];

		//ImVec4 tmp;
		//tmp.x = tmp.y = tmp.z = tmp.w = fade.value( );

		const auto color = ImLerp(from, to, fade.value( ));
		return ImGui::ColorConvertFloat4ToU32(color);
	}
};

selectable::selectable( )
{
	data_ = std::make_unique<data_type>( );

#define LOG_COLOR(msg) CHEAT_CONSOLE_LOG(std::format(msg ": {}->{}",(uint8_t)d.clr_last,(uint8_t)d.clr_curr))

	this->add_hovered_callback([](const callback_data& data, [[maybe_unused]] const callback_state& state)
	{
		auto& d = *static_cast<selectable*>(data.caller)->data_;
		d.set_color(data_type::COLOR_HOVERED);
		LOG_COLOR("Hovered");
	}, false);
	this->add_unhovered_callback([](const callback_data& data, const callback_state& state)
	{
		auto& sel = *static_cast<selectable*>(data.caller);
		auto& d   = *sel.data_;
		d.set_color(sel.selected( ) ? data_type::COLOR_SELECTED : data_type::COLOR_DEFAULT);
		LOG_COLOR("Unhovered");
	}, false);
	this->add_held_callback([](const callback_data& data, [[maybe_unused]] const callback_state& state)
	{
		auto& d = *static_cast<selectable*>(data.caller)->data_;
		d.set_color(data_type::COLOR_HELD);
		LOG_COLOR("Held");
	}, false);
	this->add_unheld_callback([](const callback_data& data, [[maybe_unused]] const callback_state& state)
	{
		auto& d = *static_cast<selectable*>(data.caller)->data_;
		d.set_color(d.clr_last);
		LOG_COLOR("Unheld");
	}, false);
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
	bb.Max.x += spacing_x - spacing_L;
	bb.Max.y += spacing_y - spacing_U;

	const auto id = this->get_id(window);

	/*if (!ImGui::ItemAdd(bb, id))
		return;*/
	if (!bb.Overlaps(window->ClipRect))
		return;

	//data_->reset_colors( );
	//
	//auto& [bg_color,fade/*,bg_color_override*/] = *data_;

	auto cb_data = callback_data({this, id});
	this->invoke_callbacks(bb, cb_data);

	this->Animate( ); //DEPRECATED

#if 0
	if (this->Animate( ))
	{
		auto clr = bg_color.has_value( ) ? ImGui::ColorConvertU32ToFloat4(*bg_color) : ImGui::GetStyleColorVec4(ImGuiCol_Header);
		clr.w *= this->Anim_value( );
		bg_color/*_override*/ = ImGui::ColorConvertFloat4ToU32(clr);
	}
	else if (this->selected( ))
	{
		bg_color = ImGui::GetColorU32(ImGuiCol_Header);
		//--
	}
#endif

#if 0
	const auto animating = this->Animate( );
	if (!bg_color.has_value( ))
	{
		if (animating)
		{
			auto clr = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			clr.w *= this->Anim_value( );
			bg_color/*_override*/ = ImGui::ColorConvertFloat4ToU32(clr);
		}
		else if (this->selected( ))
		{
			bg_color = ImGui::GetColorU32(ImGuiCol_Header);
			//--
		}
	}
#endif

	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	//if (bg_color.has_value( ))
	//	window->DrawList->AddRectFilled(bb.Min, bb.Max, *bg_color);

	window->DrawList->AddRectFilled(bb.Min, bb.Max, data_->get_color( ));

	this->render_text(window, text_pos);

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

	//text::render( );
}
