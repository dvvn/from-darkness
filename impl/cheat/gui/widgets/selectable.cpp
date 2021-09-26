// ReSharper disable CppMemberFunctionMayBeConst
#include "selectable.h"

#include "cheat/core/console.h"
#include "cheat/gui/tools/animator.h"

#include <nstd/overload.h>

#include <imgui_internal.h>

#include <array>
#include <format>
#include <functional>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable_bg::data_type
{
	state current_state = STATE_IDLE;
	ImVec4 current_color;

	struct
	{
		ImVec4 def, selected, hovered, held;

		ImVec4& operator[](state s)
		{
			switch (s)
			{
				case STATE_IDLE: return def;
				case STATE_SELECTED: return selected;
				case STATE_HOVERED: return hovered;
				case STATE_HELD: return held;
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
			color_modifier->update_end(new_color);
		}
		current_state = s;
	}

	data_type()
	{
		//temp solution
		auto& sc = ImGui::GetStyle( ).Colors;

		colors.def      = {};
		colors.selected = sc[ImGuiCol_Header];
		colors.hovered  = sc[ImGuiCol_HeaderHovered];
		colors.held     = sc[ImGuiCol_HeaderActive];
	}
};

selectable_bg::selectable_bg()
{
	data_ = std::make_unique<data_type>( );

	using w = two_way_callback::ways;

	this->add_hovered_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		data.set_state(STATE_HOVERED);
	}), w::WAY_TRUE);
	this->add_hovered_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		data.set_state(this->selected( ) ? STATE_SELECTED : STATE_IDLE);
	}), w::WAY_FALSE);

	this->add_held_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		data.set_state(STATE_HELD);
	}), w::WAY_TRUE);
	this->add_held_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		data.set_state(STATE_HOVERED);
	}), w::WAY_FALSE);

	this->add_selected_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		if (data.current_state != STATE_IDLE)
			return;
		data.set_state(STATE_SELECTED);
	}), w::WAY_TRUE);
	this->add_selected_callback(make_callback_info([=]
	{
		auto& data = *this->data_;
		if (data.current_state != STATE_SELECTED)
			return;
		data.set_state(STATE_IDLE);
	}), w::WAY_FALSE);
}

selectable_bg::~selectable_bg()                                   = default;
selectable_bg::selectable_bg(selectable_bg&&) noexcept            = default;
selectable_bg& selectable_bg::operator=(selectable_bg&&) noexcept = default;

void selectable_bg::set_background_color_modifier(std::unique_ptr<animation_property<ImVec4>>&& mod)
{
	auto target = std::make_unique<animation_property_target_external<ImVec4>>( );
	target->set_target(data_->current_color);
	mod->set_target(std::move(target));
	data_->color_modifier = std::move(mod);
}

bool selectable_bg::render(ImGuiWindow* window, ImRect& bb, ImGuiID id, bool outer_spacing)
{
	if (outer_spacing)
	{
		const auto& style = ImGui::GetStyle( );

		const auto spacing_x = /*span_all_columns ? 0.0f :*/ style.ItemSpacing.x;
		const auto spacing_y = style.ItemSpacing.y;
		const auto spacing_L = IM_FLOOR(spacing_x * 0.50f);
		const auto spacing_U = IM_FLOOR(spacing_y * 0.50f);
		bb.Min.x -= spacing_L;
		bb.Min.y -= spacing_U;
		bb.Max.x += spacing_x - spacing_L;
		bb.Max.y += spacing_y - spacing_U;
	}

	/*if (!ImGui::ItemAdd(bb, id))
		return;*/
	if (!bb.Overlaps(window->ClipRect))
		return false;

	this->invoke_button_callbacks(bb, id);
	this->render_background(window, bb);

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
	return true;
}

void selectable_bg::render_background(ImGuiWindow* window, ImRect& bb)
{
	if (data_->color_modifier)
		data_->color_modifier->update( );
	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(data_->current_color));
}

//--------------

struct selectable::data_type
{
};

selectable::selectable()
{
	data_ = std::make_unique<data_type>( );
}

selectable::~selectable()                                = default;
selectable::selectable(selectable&&) noexcept            = default;
selectable& selectable::operator=(selectable&&) noexcept = default;

void selectable::render()
{
	const auto window = ImGui::GetCurrentWindow( );

	auto bb = this->make_rect(window);
	ImGui::ItemSize(bb.GetSize( ));
	const auto text_pos = bb.Min;

	if (!selectable_bg::render(window, bb, this->get_id(window), true))
		return;

	this->render_text(window, text_pos);
}
