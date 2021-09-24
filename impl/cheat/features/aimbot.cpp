#include "aimbot.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "cheat/gui/imgui context.h"
#include "cheat/gui/widgets/checkbox.h"

#include "cheat/core/services loader.h"
#include "cheat/gui/tools/cached_text.h"

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
	checkbox   cb,  cb2, cb3;
	selectable sel, sel2;

	void init_gui()
	{
		constexpr auto make_ptr = []<class T>(T& obj)
		{
			return std::make_unique<T>(obj);
		};

		using namespace std::chrono_literals;

		auto font_cfg        = gui::fonts_builder_proxy::default_font_config( );
		font_cfg->SizePixels = 20;

		auto test_font = gui::imgui_context::get_ptr( )->fonts( ).add_font_from_ttf_file(R"(C:\Windows\Fonts\arial.TTF)", std::move(font_cfg));

		auto anim_sample = animation_property_linear<ImVec4>( );
		anim_sample.set_duration(300ms);

		cb3.set_font(ImGui::GetDefaultFont( ));
		cb3.set_label("asdaewbabwabebe");
		cb3.set_background_color_modifier(make_ptr(anim_sample));
		cb3.set_check_color_modifier(make_ptr(anim_sample));

		cb.set_font(test_font);
		cb.set_label(u8"test йцук 網站有中 ");
		cb.set_background_color_modifier(make_ptr(anim_sample));
		cb.set_check_color_modifier(make_ptr(anim_sample));
		//check_anim->set_time(animator::default_time * 3);

		cb.select( );

		//--

		cb2.set_font(test_font);
		cb2.set_label(u8"sfsdfsf кцфиции333");
		cb2.set_background_color_modifier(make_ptr(anim_sample));
		cb2.set_check_color_modifier(make_ptr(anim_sample));

		sel.set_font(test_font);
		sel.set_label(u8"ерарапр 123123123");
		sel.set_background_color_modifier(make_ptr(anim_sample));

		sel2.set_font(ImGui::GetDefaultFont( ));
		sel2.set_label(u8"е45е4е аааааaaaaa 88888888");
		sel2.set_background_color_modifier(make_ptr(anim_sample));
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
	impl_->cb3.render( );
	ImGui::Text("1");
	impl_->cb.render( );
	ImGui::SameLine( );
	ImGui::Text("1.5");
	impl_->cb2.render( );
	ImGui::Text("2");
	ImGui::SameLine( );
	ImGui::Text("3");
	impl_->sel2.render( );
	ImGui::SameLine();
	impl_->sel.render( );
}

CHEAT_REGISTER_SERVICE(aimbot);
