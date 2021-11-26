#pragma once
#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/button_info.h"
#include "cheat/gui/tools/cached_text.h"
#include "cheat/gui/tools/imgui_id.h"

#include <nstd/smooth_value.h>
#include <nstd/math.h>

#include <imgui_internal.h>

//affected only if animator owns value
#define CHEAT_GUI_SLIDER_WRITE_INSTANT

namespace cheat::gui::widgets
{
	template <typename T>
	struct slider_input_data
	{
		using value_type = T;

		slider_input_data(const T& value, const T& min, const T& max, const T& step)
			: value(value),
			  min(min),
			  max(max),
			  step(step)
		{
			runtime_assert(max > min);
			runtime_assert(value >= min && value <= max);
		}

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

		const auto window = ImGui::GetCurrentWindow( );
		const auto& style = ImGui::GetStyle( );

		const auto w = ImGui::CalcItemWidth( );

		const auto frame_bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, text.label_size.y + style.FramePadding.y * 2.0f));
		const auto total_bb = ImRect(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x + text.label_size.x, 0.0f));

		ImGui::ItemSize(total_bb);
		if (!total_bb.Overlaps(window->ClipRect))
			return tools::button_state::UNKNOWN;

		//----

		using Tfloat = std::conditional_t<std::floating_point<T>, T, decltype(std::declval<ImVec2>( ).x)>;
		const auto grab_size = [&]
		{
			const auto slider_sz = frame_bb.GetWidth( );
			const auto v_range   = nstd::to_integer(static_cast<Tfloat>(in.max - in.min) / static_cast<Tfloat>(in.step));
			const auto grab_sz   = std::max(slider_sz / v_range, style.GrabMinSize);
			return grab_sz;
		}( );

		const auto current_value = !slide_animation ? in.value : slide_animation->get_target( )->get( );

		const auto grab_offset = [&]
		{
			//const auto safe_max = in.max - in.step;
			//const auto value    = safe_max < in.value ? safe_max : in.value;

			const auto value_offset_normalized = nstd::normalize<float>(current_value, in.min, in.max);
			return frame_bb.Min.x + (frame_bb.GetWidth( ) - grab_size) * value_offset_normalized;;
		}( );

		const auto grab_bb = ImRect({grab_offset, frame_bb.Min.y}, {grab_offset + grab_size, frame_bb.Max.y});
		const auto id      = tools::imgui_id(text);

		const auto get_mouse_offset = [&]
		{
			const auto val = static_cast<Tfloat>(ImGui::GetIO( ).MousePos.x);
			const auto min = static_cast<Tfloat>(frame_bb.Min.x);
			const auto max = static_cast<Tfloat>(frame_bb.Max.x);

			return nstd::normalize(val, min, max);
		};
		using animation_state = nstd::smooth_object_state;
		const auto set_new_value = [&](bool animate, bool external = false)-> void
		{
			T new_value_fixed;

			if (external)
			{
				new_value_fixed = std::clamp(nstd::clamp_by(in.value, in.step), in.min, in.max);
			}
			else
			{
				using nstd::scale_mode;
				const auto temp = nstd::scale<scale_mode::RIGHT, Tfloat>(get_mouse_offset( ), in.min, in.max, in.step);

				new_value_fixed = static_cast<T>(temp);
#ifdef CHEAT_GUI_SLIDER_WRITE_INSTANT
				in.value = new_value_fixed;
#endif
			}

			if (!slide_animation)
			{
				in.value = new_value_fixed;
			}
			else
			{
				if (animate)
				{
					if (!external || slide_animation->get_target( )->own( ))
					{
						slide_animation->set_new_range(new_value_fixed);
					}
					else //we have no temporary value to animate
					{
						in.value = new_value_fixed;
						slide_animation->inverse( );
						slide_animation->set_end(new_value_fixed);
					}
				}
				else
				{
					switch (slide_animation->get_state( ))
					{
						case animation_state::RUNNING:
							slide_animation->set_new_range(new_value_fixed);
							break;
						case animation_state::IDLE:
							slide_animation->set_start(in.value);
							slide_animation->set_end(new_value_fixed);
							slide_animation->get_target( )->get( ) = new_value_fixed;
							break;
					}
				}
			}

			//----
		};

		//---

		const auto state = button_behavior(frame_bb, id);
		using tools::button_state;
		switch (state)
		{
			case button_state::UNKNOWN:
				return state;
			case button_state::PRESSED:
#ifndef CHEAT_GUI_SLIDER_WRITE_INSTANT
				if (slide_animation && slide_animation->get_target()->own() && !slide_animation->active( ))
					in.value = slide_animation->get_target( )->get( );
#endif
				break;
			case button_state::HELD:
				set_new_value(!ImGui::IsMouseHoveringRect(grab_bb.Min, grab_bb.Max, false));
				break;
			case button_state::HELD_ACTIVE:
				set_new_value(false);
				break;
			case button_state::IDLE:
				set_new_value(true, true);
				break;
		}

		if (slide_animation && slide_animation->update( ))
		{
#ifndef CHEAT_GUI_SLIDER_WRITE_INSTANT
			if (slide_animation->get_state( ) == animation_state::FINISHED && slide_animation->get_target( )->own( ) )
			{
				in.value = slide_animation->get_target( )->get( );
			}
#endif
		}

		//---

		const auto& colors  = style.Colors;
		const auto bg_color = get_button_color(state, true
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
