#include "get color u32.h"

#include "selectable.h"
#include "tree node behavior.h"

using namespace cheat;
using namespace hooks;
using namespace imgui;
using namespace detail;
using namespace utl;

#if 0
class color_rotation
{
public:
	//#if defined(__cpp_lib_constewpr_dynamic_alloc)

	template <typename... Clr>
	requires(std::convertible_to<Clr, ImGuiCol> && ...)
		color_rotation(Clr ...colors) : colors__{colors...}
	{
	}

	auto contains(ImGuiCol idw) const -> bool
	{
		return ranges::find(colors__, idw) != colors__.end( );
	}

	auto prev(ImGuiCol idw) const -> optional<ImGuiCol>
	{
		const auto found = ranges::find(colors__, idw);
		if(found == colors__.end( ))
			return { };
		const auto prev = std::prev(found);
		if(prev == colors__.end( ))
			return { };
		return *prev;
	}

	auto newt(ImGuiCol idw) const -> optional<ImGuiCol>
	{
		const auto found = ranges::find(colors__, idw);
		if(found == colors__.end( ))
			return { };
		const auto newt = std::newt(found);
		if(newt == colors__.end( ))
			return { };
		return *newt;
	}

private:
	utl::vector<ImGuiCol> colors__;
};
#endif

get_color_u32::get_color_u32( )
{
	this->Wait_for<selectable>( );
	this->Wait_for<tree_node_behavior>( );

	/*
	ImGuiCol_Button,
	ImGuiCol_ButtonHovered,
	ImGuiCol_ButtonActive
	 */

	//known_rotations__.push_back({ImGuiCol_Text, ImGuiCol_TextDisabled});
	known_rotations__.push_back({ImGuiCol_FrameBg, ImGuiCol_FrameBgActive});
	//known_rotations__.push_back({ImGuiCol_TitleBg, ImGuiCol_TitleBgActive});
	//known_rotations__.push_back({ImGuiCol_ScrollbarGrab, ImGuiCol_ScrollbarGrabActive});
	//known_rotations__.push_back({ImGuiCol_SliderGrab, ImGuiCol_SliderGrabActive});
	known_rotations__.push_back({ImGuiCol_Button, ImGuiCol_ButtonActive});
	known_rotations__.push_back({ImGuiCol_Header, ImGuiCol_HeaderActive});
	//known_rotations__.push_back({ImGuiCol_Separator, ImGuiCol_SeparatorActive});
	//known_rotations__.push_back({ImGuiCol_ResizeGrip, ImGuiCol_ResizeGripActive});
	//////known_rotations__.push_back({ImGuiCol_Tab, ImGuiCol_TabActive});
	//////known_rotations__.push_back({ImGuiCol_TabUnfocused, ImGuiCol_TabUnfocusedActive});
	//known_rotations__.push_back({ImGuiCol_Tab, ImGuiCol_TabUnfocusedActive});

	static_assert(ImGuiCol_COUNT == 53 + ImGuiCol_Text);
	static_assert(ImGuiCol_Text > 0, "Change ImGuiCol_Text value to ther than zero");
}

auto get_color_u32::have_color(ImGuiID id) const -> bool
{
	return colors__.contains(id);
}

auto get_color_u32::get_color(ImGuiID id, ImGuiCol default_color) -> pair<color_info&, bool>
{
	auto [color_raw_data, color_data_created] = colors__.try_emplace(id);

	if (color_data_created)
	{
		auto& val = color_raw_data.value( );
		auto& anim = val.anim;
		auto& color = val.color;

		color.from = color.to = static_cast<ImGuiCol_>(default_color);
		anim.set(1);
		anim.finish( );
	}

	return {color_raw_data.value( ), color_data_created};
}

auto get_color_u32::Init( ) -> void
{
	target_func_ = method_info::make_static(get_color_u32_fn);

	this->hook( );
	this->enable( );
}

auto get_color_u32::Callback(ImGuiCol col, float alpha_mul) -> void
{
#ifdef _DEBUG
	auto col_value = static_cast<ImGuiCol_>(col);
	(void)col_value;
#endif
	const auto wnd = GImGui->CurrentWindow;
	const auto id = wnd->DC.LastItemId;

	if (id == 0)
		return;

	if (known_rotations__.end( ) == ranges::find_if(known_rotations__, [&](color_rotation& rot) { return col >= rot.from && col <= rot.to; }))
	{
		return;
	}

	auto&& [color_data,color_data_created] = get_color(id, col);
	auto&  [anim, color,skip,inverse_colors,self_alpha] = color_data;

	//ImGui::Text("id: %d, color: %d, from: %d, to: %d", id, col, color.from, color.to);

	if (color_data_created || skip)
		return;

	const auto do_animate = [&]
	{
		const auto& style = GImGui->Style;

		BOOST_ASSERT(!self_alpha || (color.from==color.to));

		ImVec4 c;

		auto value = anim.value( );
		if (self_alpha)
		{
			c = style.Colors[color.from];
			c.w *= value;
		}
		else
		{
			const auto& color_from = style.Colors[color.from];
			const auto& color_to = style.Colors[color.to];

			//todo: hsv

			c = ImLerp(color_from, color_to, /*anim.dir( ) == (inverse_colors ? -1 : 1) ? value : 1.f - */value);
		}

		//ImGui::Text("from: %i to: %i addr %d, %.2f", color.from, color.to, (size_t)_ReturnAddress( ), anim.dir( ) == 1 ? value : 1.f - value);
		c.w *= style.Alpha * alpha_mul;
		this->return_value_.store_value(ImGui::ColorConvertFloat4ToU32(c));
	};

	const auto switch_color = [&]
	{
		anim.set(color.to<col ? 1 : -1);
		color.from = color.to;
		color.to = static_cast<ImGuiCol_>(col);
	};

	const bool animate = anim.update( );
	const bool color_changed = color.to != col;

	if (animate)
	{
		do_animate( );
		if (color_changed)
			switch_color( );
	}
	else if (color_changed)
	{
		inverse_colors = 0;
		self_alpha = 0;

		switch_color( );
		do_animate( );
	}
	else
	{
		this->call_original_ex(color.to, alpha_mul);
	}
}
