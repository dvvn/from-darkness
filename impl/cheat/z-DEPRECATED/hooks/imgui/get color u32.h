#pragma once

#include "cheat/gui/animator.h"
#include "cheat/services/async_service.h"

namespace cheat::hooks::imgui
{
	namespace detail
	{
		//using color_rotation = utl::vector<ImGuiCol>;
		struct color_rotation
		{
			ImGuiCol_ from, to;
		};
		struct color_info
		{
			gui::animator  anim;
			color_rotation color;

			bool skip = false;
			bool inverse_colors = false;
			bool self_alpha=false;
		};
		constexpr ImU32 (*get_color_u32_fn)(ImGuiCol id, float) = ImGui::GetColorU32;
	}

	class get_color_u32 final: public services::async_service_shared<get_color_u32>,
							   public decltype(detect_hook_holder(detail::get_color_u32_fn))
	{
	public:
		get_color_u32( );

		auto have_color(ImGuiID id) const -> bool;
		auto get_color(ImGuiID id, ImGuiCol default_color) -> utl::pair<detail::color_info&, bool>;

	protected:
		auto Init( ) -> void override;
		auto Callback(ImGuiCol col, float alpha_mul) -> void override;

	private:
		utl::vector<detail::color_rotation>             known_rotations__;
		utl::unordered_map<ImGuiID, detail::color_info> colors__;
	};
}
