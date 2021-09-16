#include "aimbot.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "cheat/gui/imgui context.h"
#include "cheat/gui/tools/string wrapper.h"
#include "cheat/gui/widgets/checkbox.h"

#include "cheat/core/services loader.h"
#include "cheat/gui/tools/animator.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <functional>
#include <filesystem>
#include <fstream>

using namespace cheat::features;

using namespace cheat::gui::tools;
using namespace cheat::gui::objects;
using namespace cheat::gui::widgets;

struct aimbot::impl
{
	checkbox cb;

	void init_gui( )
	{
		auto fonts = gui::imgui_context::get_ptr( )->fonts( );

		auto font_cfg        = fonts.default_font_config( );
		font_cfg->SizePixels = 20;
		auto test_font       = fonts.add_font_from_ttf_file(R"(C:\Windows\Fonts\arial.ttf)", std::move(font_cfg));
		fonts                = { };
		cb.set_font(test_font);
		cb.set_label("test");
		auto bg_colors = init_selectable_colors<selectable_bg_colors_fade>(ImGuiCol_FrameBg, ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive);
		add_default_selectable_callbacks(&cb, bg_colors.get( ));
		cb.set_bg_colors(std::move(bg_colors));
		auto check_bg_color = init_selectable_colors<selectable_bg_colors_fade>({ }, ImGuiCol_CheckMark, { }, { });
		check_bg_color->fade( ).set_time(animator::default_time * 3);
		cb.set_check_colors(std::move(check_bg_color));

		cb.add_pressed_callback({[&](const callback_data& data, const callback_state& state)
		{
			cb.toggle({&cb});
		}}, two_way_callback::WAY_TRUE);
		cb.add_selected_callback({[&](const callback_data& data, const callback_state& state)
		{
			auto& clr = *cb.get_check_colors( );
			clr.change_color(selectable_bg_colors_base::COLOR_SELECTED);
		}}, two_way_callback::WAY_TRUE);
		cb.add_selected_callback({[&](const callback_data& data, const callback_state& state)
		{
			auto& clr = *cb.get_check_colors( );
			clr.change_color(selectable_bg_colors_base::COLOR_DEFAULT);
		}}, two_way_callback::WAY_FALSE);
		cb.select({&cb});
	}
};

aimbot::aimbot( )
{
	impl_ = std::make_unique<impl>( );
	this->wait_for_service<gui::imgui_context>( );
}

aimbot::~aimbot( )                           = default;
aimbot::aimbot(aimbot&&) noexcept            = default;
aimbot& aimbot::operator=(aimbot&&) noexcept = default;

void aimbot::save(json& in) const
{
}

void aimbot::load(const json& out)
{
}

cheat::service_base::load_result aimbot::load_impl( )
{
	impl_->init_gui( );
	co_return service_state::loaded;
}

void aimbot::render( )
{
	impl_->cb.render( );

	ImGui::Text("1");
	ImGui::SameLine( );
	ImGui::Text("1.5");
	ImGui::Text("2");
}

CHEAT_REGISTER_SERVICE(aimbot);
