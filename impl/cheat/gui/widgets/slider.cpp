#include "slider.h"

#include "cheat/gui/tools/cached_text.h"
#include "cheat/gui/tools/imgui_id.h"
#include "cheat/gui/tools/animation_tools.h"

#include <functional>

#include <nstd/runtime assert.h>
#include <nstd/smooth_value.h>

#include <imgui_internal.h>

using namespace cheat::gui;
using namespace widgets;

#if 0

std::_Slider_data std::lerp(const _Slider_data& a, const _Slider_data& b, float fl)
{
	return std::visit([&]<typename Q>(const slider_input_value<Q>& _a)-> _Slider_data
	{
		const auto& _b = std::get<slider_input_value<Q>>(b);
		auto val       = _a;
		val.value      = (Q)(std::lerp((float)_a.value, (float)_b.value, fl));
		return val;
	}, a);
}


tools::button_state widgets::slider(const tools::cached_text& text, slider_input_data& value,
									nstd::smooth_value_base<ImVec4>* bg_animation
								  , nstd::smooth_value_base<slider_input_data>* slide_animation)
{
	//ImGui::SliderAngle()
	//ImGui::SliderBehavior( )

	const auto window = ImGui::GetCurrentWindow( );
	const auto& style = ImGui::GetStyle( );

	const float w = ImGui::CalcItemWidth( );

	const ImVec2& label_size = text.label_size;
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	ImGui::ItemSize(total_bb);
	if (!total_bb.Overlaps(window->ClipRect))
		return tools::button_state::UNKNOWN;

	// ReSharper disable once CppInconsistentNaming
	const auto _Visit = [&]<typename Fn,typename ...Args>(Fn&& fn, Args&&...args)
	{
		if constexpr (sizeof...(Args) > 0)
			return std::visit(std::bind_front(fn, std::forward<Args>(args)...), value);
		else
			return std::visit(fn, value);
	};

	//IsMouseHoveringRect

	constexpr auto clamp_value = []<typename T>(T val, T step)-> T
	{
		return val;
		//return std::floor(val / step) * val;
	};

	const auto mouse_offset = [&]
	{
		const auto val = ImGui::GetIO( ).MousePos.x;
		const auto min = frame_bb.Min.x;
		const auto max = frame_bb.Max.x;

		const auto normalized = (val - min) / (max - min);
		return normalized;
	};
	const auto value_offset = [&]()-> float
	{
		const auto fn = [&]<typename T>(slider_input_value<T>& val)-> float
		{
			T v;
			if (slide_animation && slide_animation->get_state( ) == nstd::smooth_object_base::state::RUNNING) //&& !external
				v = std::get<T>(slide_animation->get_target( )->get_value( ));
			else
				v = *val.value;

			auto normalized = (float)(v - val.min) / (float)(val.max - val.min);
			return clamp_value(normalized, (float)val.step);
		};

		return _Visit(fn);
	};

	const auto grab_offset = frame_bb.Min.x + frame_bb.GetWidth( ) * value_offset( );
	const ImRect grab_bb({grab_offset, frame_bb.Min.y}, {grab_offset + 4/*grab size*/, frame_bb.Max.y}); //test
	const ImGuiID id = tools::imgui_id(value);

	const auto fix_slider_pos = [&]<typename T>(slider_input_value<T>& val)
	{
		if (slide_animation == nullptr)
			return;
		if (slide_animation->get_target( )->id( ) == nstd::smooth_value_base<slider_value>::target_external::id_value)
			return;

		auto value_fixed = clamp_value(*val.value, val.step);
		*val.value       = value_fixed;
		slide_animation->update_end((value_fixed));
	};
	const auto set_new_value = [&]<typename T>(bool animate, slider_input_value<T>& val)
	{
		T new_value = val.min + (val.max - val.min) * mouse_offset( );

		auto new_value_fixed = clamp_value(new_value, val.step);

		if (animate && slide_animation != nullptr)
			slide_animation->update_end((new_value_fixed));
		else
			*val.value = new_value_fixed;
	};

	const auto state = tools::button_behavior(frame_bb, id);
	using bs = tools::button_state;
	switch (state)
	{
		case bs::UNKNOWN:
			return bs::UNKNOWN;
		case bs::HELD:
			_Visit(set_new_value, !ImGui::IsMouseHoveringRect(grab_bb.Min, grab_bb.Max, false));
			break;
		case bs::HELD_ACTIVE:
			//_Visit(set_new_value, false);
			break;
		default:
			//_Visit(fix_slider_pos);
			break;
	}

	if (slide_animation && slide_animation->update( ))
	{
	}

	const auto& colors  = style.Colors;
	const auto bg_color = get_button_color(state, true
										 , colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBgHovered], colors[ImGuiCol_FrameBgActive]
										 , bg_animation);

	//----

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_color, true, style.FrameRounding);

	// Render grab
	//if (grab_bb.Max.x > grab_bb.Min.x)
	window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, ImGui::GetColorU32(/*g.ActiveId == id*/0 ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

	text.render(window->DrawList, ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_Text));

	return state;
}
#endif