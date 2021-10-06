#pragma once
#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/button_info.h"
#include "cheat/gui/tools/cached_text.h"
#include "cheat/gui/tools/imgui_id.h"

#include <nstd/smooth_value.h>

#include <imgui_internal.h>

namespace nstd
{
	template <std::floating_point T>
	constexpr T normalize(T value, T min, T max)
	{
		if (value <= min)
			return 0;
		if (value >= max)
			return 1;

		return (value - min) / (max - min);
	}

	constexpr auto qeqwe = sizeof(long double);

	template <std::integral T>
	constexpr auto to_floating(T value)
	{
		// ReSharper disable once CppTooWideScopeInitStatement
		constexpr auto sz = sizeof(T);

		if constexpr (sz <= sizeof(float))
			return static_cast<float>(value);
		else if constexpr (sz == sizeof(double))
			return static_cast<double>(value);
		else
			return static_cast<long double>(value);
	}

	template <std::floating_point T>
	constexpr T to_floating(T value)
	{
		return value;
	}

	enum class scale_mode:uint8_t
	{
		RIGHT
	  , LEFT
	  , AUTO
	};

	template <scale_mode Mode = scale_mode::AUTO, typename T>
	constexpr T scale(T diff, T left, T right)
	{
		if constexpr (Mode == scale_mode::RIGHT)
			return left + (right - left) * diff;
		else if constexpr (Mode == scale_mode::LEFT)
			return right - (right - left) * diff;
		else if constexpr (Mode == scale_mode::AUTO)
		{
			return left < right
					   ? scale<scale_mode::RIGHT>(diff, left, right)
					   : scale<scale_mode::LEFT>(diff, right, left);
		}
		else
		{
			static_assert(__FUNCTION__": unknown mode");
			return left;
		}
	}

	template <scale_mode Mode = scale_mode::AUTO, typename T>
		requires(std::is_arithmetic_v<T>)
	constexpr T scale(T diff, T left, T right, T step)
	{
		auto val = scale<Mode>(diff, left, right);

		if constexpr (std::is_floating_point_v<T>)
		{
			auto ideal = std::floor(val / step);
			return ideal * step;
		}
		else
		{
			auto rem = val % step;
			return val - rem;
		}
	}
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
			const float v_range  = (in.max - in.min) / in.step;
			const auto grab_sz   = std::max(slider_sz / v_range, style.GrabMinSize);
			return grab_sz;
		}( );
		const auto grab_offset = [&]
		{
			const auto safe_max = in.max - in.step;
			const auto value    = safe_max < in.value ? safe_max : in.value;

			const auto value_offset_normalized = nstd::normalize<float>(value, in.min, in.max);
			return frame_bb.Min.x + frame_bb.GetWidth( ) * value_offset_normalized;;
		}( );

		const auto grab_bb = ImRect({grab_offset, frame_bb.Min.y}, {grab_offset + grab_size, frame_bb.Max.y});
		const auto id      = tools::imgui_id(text);

		const auto get_mouse_offset = [&]()-> float
		{
			const auto val = ImGui::GetIO( ).MousePos.x;
			const auto min = frame_bb.Min.x;
			const auto max = frame_bb.Max.x;

			return nstd::normalize(val, min, max);
		};

		const auto set_new_value = [&](bool animate)
		{
			using nstd::scale_mode;
			auto new_value_fixed = nstd::scale<scale_mode::RIGHT, float>(get_mouse_offset( ), in.min, in.max, in.step);

			if (!slide_animation)
				in.value = new_value_fixed;
			else if (animate)
				slide_animation->update_end(new_value_fixed);
			else
			{
				using state = nstd::smooth_object_base::state;
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

		const auto state = button_behavior(frame_bb, id);
		using tools::button_state;
		switch (state)
		{
			case button_state::UNKNOWN:
				return state;
			case button_state::HELD:
				set_new_value(!ImGui::IsMouseHoveringRect(grab_bb.Min, grab_bb.Max, false));
				break;
			case button_state::HELD_ACTIVE:
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
