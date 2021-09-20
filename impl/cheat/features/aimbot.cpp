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

	void init_gui()
	{
		using namespace std::chrono_literals;

		auto font_cfg        = gui::fonts_builder_proxy::default_font_config( );
		font_cfg->SizePixels = 20;

		auto test_font = gui::imgui_context::get_ptr( )->fonts( ).add_font_from_ttf_file(R"(C:\Windows\Fonts\ARIALUNI.TTF)", std::move(font_cfg));
		cb.set_font(test_font);
		cb.set_label(u8"test йцук 網站有中 ");

		auto bg_anim = std::make_unique<animation_property_linear<ImVec4>>( );
		bg_anim->set_duration(300ms);
		cb.set_background_color_modifier(std::move(bg_anim));
		auto check_anim = std::make_unique<animation_property_linear<ImVec4>>( );
		check_anim->set_duration(300ms);
		cb.set_check_color_modifier(std::move(check_anim));
		//check_anim->set_time(animator::default_time * 3);

		cb.select( );
	}
};

aimbot::aimbot()
{
	this->wait_for_service<gui::imgui_context>( );
}

aimbot::~aimbot()                            = default;
aimbot::aimbot(aimbot&&) noexcept            = default;
aimbot& aimbot::operator=(aimbot&&) noexcept = default;

void aimbot::save(json& in) const
{
}

void aimbot::load(const json& out)
{
}

cheat::service_base::load_result aimbot::load_impl()
{
	impl_ = std::make_unique<impl>( );
	impl_->init_gui( );
	co_return service_state::loaded;
}

void aimbot::render()
{
	impl_->cb.render( );

	ImGui::Text("1");
	ImGui::SameLine( );
	ImGui::Text("1.5");
	ImGui::Text("2");
}

CHEAT_REGISTER_SERVICE(aimbot);
