#pragma once
#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/button_info.h"
#include "cheat/gui/tools/cached_text.h"
#include "cheat/gui/tools/imgui_id.h"

#include <nstd/smooth_value.h>

#include <imgui_internal.h>

#include <variant>
#include <typeinfo>

#define CHEAT_GUI_SLIDER_DATA_TYPES\
	uint8_t, uint16_t, uint32_t, uint64_t,\
	int8_t, int16_t, int32_t, int64_t,\
	float, double, long double

struct ImVec4;
using ImGuiSliderFlags = int;

namespace cheat::gui::tools
{
	class cached_text;
}

namespace cheat::gui::widgets
{
	//ImGuiSliderFlags_Logarithmic unsupported
	//ImGuiSliderFlags_NoRoundToFormat must be set

	template <typename T>
	struct slider_input_data
	{
		using value_type = T;

		T value;
		T min, max;
		T step;
	};

	template <typename T>
		requires(std::is_arithmetic_v<T>)
	tools::button_state slider(const tools::cached_text& text, slider_input_data<T>& in
							 , nstd::smooth_value_base<ImVec4>* bg_animation = nullptr
							 , nstd::smooth_value_base<T>* slide_animation   = nullptr)
	{
		//todo: separate slider & text like checkbox do
		//move magic calculations to own functions

		const auto window = ImGui::GetCurrentWindow( );
		const auto& style = ImGui::GetStyle( );

		const auto w = ImGui::CalcItemWidth( );

		const auto frame_bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, text.label_size.y + style.FramePadding.y * 2.0f));
		const auto total_bb = ImRect(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x + text.label_size.x, 0.0f));

		ImGui::ItemSize(total_bb);
		if (!total_bb.Overlaps(window->ClipRect))
			return tools::button_state::UNKNOWN;

		//----

		const auto grab_size = [&]
		{
			const auto slider_sz = frame_bb.GetWidth( );
			const auto v_range   = (in.max - in.min) / in.step;
			const auto grab_sz   = std::max(slider_sz / v_range, style.GrabMinSize);
			return grab_sz;
		}( );
		const auto grab_offset = [&]
		{
			const auto safe_max = in.max - in.step;
			const auto value    = safe_max < in.value ? safe_max : in.value; //magic

			const auto value_offset_normalized = (float)(value - in.min) / (float)(in.max - in.min); //magic
			return frame_bb.Min.x + frame_bb.GetWidth( ) * value_offset_normalized;;
		}( );

		const ImRect grab_bb({grab_offset, frame_bb.Min.y}, {grab_offset + grab_size, frame_bb.Max.y});
		const ImGuiID id = tools::imgui_id(text);

		const auto mouse_offset = [&]
		{
			const auto val = ImGui::GetIO( ).MousePos.x;
			const auto min = frame_bb.Min.x;
			const auto max = frame_bb.Max.x;

			const auto normalized = (val - min) / (max - min);
			return std::clamp<float>(normalized, 0, 1);
		};

		const auto set_new_value = [&](bool animate)
		{
			T new_value = in.min + (in.max - in.min) * mouse_offset( ); //magic

			auto new_value_fixed = std::floor(new_value / in.step) * in.step; //magic

			using state = nstd::smooth_object_base::state;

			if (!slide_animation)
				in.value = new_value_fixed;
			else if (animate)
				slide_animation->update_end(new_value_fixed);
			else
			{
				switch (slide_animation->get_state( ))
				{
					case state::STARTED:
					case state::RUNNING:
						slide_animation->set_end(new_value_fixed);
						break;
					default:
						slide_animation->inverse( );
						slide_animation->set_end(new_value_fixed);
						in.value = new_value_fixed;
						break;
				}
			}
		};

		//---

		const auto state = tools::button_behavior(frame_bb, id);
		using bs = tools::button_state;
		switch (state)
		{
			case bs::UNKNOWN:
				return bs::UNKNOWN;
			case bs::HELD:
				set_new_value(!ImGui::IsMouseHoveringRect(grab_bb.Min, grab_bb.Max, false));
				break;
			case bs::HELD_ACTIVE:
				set_new_value(false);
				break;
			default:
				//_Visit(fix_slider_pos);//todo: handle externally changed value
				break;
		}

		if (slide_animation && slide_animation->update( ))
		{
		}

		//---

		const auto& colors  = style.Colors;
		const auto bg_color = tools::get_button_color(state, true
													, colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBgHovered], colors[ImGuiCol_FrameBgActive]
													, bg_animation);

		//----

		ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_color, true, style.FrameRounding);

		// Render grab
		//if (grab_bb.Max.x > grab_bb.Min.x)
		window->DrawList->AddRectFilled(grab_bb.Min, {grab_bb.Max.x, grab_bb.Max.y}
									  , ImGui::GetColorU32(/*g.ActiveId == id*/0 ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

		text.render(window->DrawList, ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_Text));

		return state;
	}
}
