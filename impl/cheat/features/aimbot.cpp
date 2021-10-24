#include "aimbot.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"

#include "cheat/gui/imgui_context.h"
#include "cheat/gui/tools/animation_tools.h"
#include "cheat/gui/tools/cached_text.h"

#include "cheat/gui/widgets/checkbox.h"
#include "cheat/gui/widgets/selectable.h"
#include "cheat/gui/widgets/slider.h"

#include <nstd/smooth_value.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>
#include <fstream>
#include <functional>

using namespace cheat::features;
using namespace cheat::gui;
using namespace cheat::gui::tools;
using namespace cheat::gui::widgets;

struct widget_bool_data : cached_text, nstd::smooth_value_linear<ImVec4>
{
	bool value = 0;
};

struct aimbot::impl
{
	impl( ) = default;

	widget_bool_data test_cb;
	nstd::smooth_value_linear<ImVec4> test_cb_check_anim;
	widget_bool_data test_selectable;

	tools::cached_text slider_text;
	slider_input_data<float> slider_data{1, 0.1f, 2.3f, 0.1f};
	nstd::smooth_value_linear<ImVec4> slider_bg_anim;
	nstd::smooth_value_linear<float> slider_anim;

	void init_gui( )
	{
		using namespace std::chrono_literals;

		auto font_cfg        = gui::fonts_builder_proxy::default_font_config( );
		font_cfg->SizePixels = 15;

		auto test_font = gui::imgui_context::get_ptr( )->fonts( ).add_font_from_ttf_file(R"(C:\Windows\Fonts\arial.TTF)", std::move(font_cfg));

		using target_internal = nstd::smooth_value_linear<ImVec4>::target_internal;
		using target_external = nstd::smooth_value_linear<ImVec4>::target_external;

		slider_bg_anim.set_duration(600ms);
		slider_bg_anim.set_target<target_internal>( );
		slider_anim.set_duration(200ms);
		slider_anim.set_target<nstd::smooth_value_linear<float>::target_internal>( );
		slider_anim.get_target( )->write_value(slider_data.value);
		slider_anim.set_start(slider_data.min);
		slider_anim.set_end(slider_data.max);
		slider_anim.inverse(  );
		slider_text.set_font(ImGui::GetDefaultFont( ));
		slider_text.set_label(std::format("custom slider: {} value", slider_anim.get_target( )->own_value( ) ? "internal" : "external"));

		test_cb.set_label(u8"hello привет 12345");
		test_cb.set_font(test_font);
		test_cb.set_duration(300ms);
		test_cb.set_target<target_internal>( );
		test_cb_check_anim.set_duration(600ms);
		test_cb_check_anim.set_target<target_internal>( );

		test_selectable.set_label(U"hello default");
		test_selectable.set_font(ImGui::GetDefaultFont( ));
		test_selectable.set_duration(300ms);
		test_selectable.set_target<target_internal>( );

		//-----
#if 0
		constexpr auto get_anim_sample = []
		{
			auto sample = std::make_unique<nstd::smooth_value_linear<ImVec4>>( );
			sample->set_duration(300ms);
			sample->write_value<target_external>( );
			return sample;
		};
		
		cb3.set_font(ImGui::GetDefaultFont( ));
		cb3.set_label("asdaewbabwabebe");
		auto cb3_anim = get_anim_sample( );
		cb3.set_background_color_modifier(get_anim_sample( ));
		cb3.set_check_color_modifier(get_anim_sample( ));

		cb.set_font(test_font);
		cb.set_label(u8"test йцук 網站有中 ");
		cb.set_background_color_modifier(get_anim_sample( ));
		cb.set_check_color_modifier(get_anim_sample( ));
		//check_anim->set_time(animator::default_time * 3);

		cb.select( );

		//--

		cb2.set_font(test_font);
		cb2.set_label(u8"sfsdfsf кцфиции333");
		cb2.set_background_color_modifier(get_anim_sample( ));
		cb2.set_check_color_modifier(get_anim_sample( ));

		sel.set_font(test_font);
		sel.set_label(u8"ерарапр 123123123");
		sel.set_background_color_modifier(get_anim_sample( ));

		sel2.set_font(ImGui::GetDefaultFont( ));
		sel2.set_label(u8"е45е4е аааааaaaaa 88888888");
		sel2.set_background_color_modifier(get_anim_sample( ));
#endif
	}
};

aimbot::aimbot( )
{
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

cheat::service_impl::load_result aimbot::load_impl( ) noexcept
{
	impl_ = std::make_unique<impl>( );
	impl_->init_gui( );
	CHEAT_SERVICE_LOADED
}

void aimbot::render( )
{
	/*impl_->cb3.render( );
	ImGui::Text("1");
	impl_->cb.render( );
	ImGui::SameLine( );
	ImGui::Text("1.5");
	impl_->cb2.render( );
	ImGui::Text("2");
	ImGui::SameLine( );
	ImGui::Text("3");
	impl_->sel2.render( );
	ImGui::SameLine( );
	impl_->sel.render( );*/

	auto& sb_data = impl_->test_selectable;
	gui::widgets::selectable2(sb_data, sb_data.value, std::addressof(sb_data));
	ImGui::Selectable("Test selectable", sb_data.value);

	auto& cb_data       = impl_->test_cb;
	auto& cb_check_anim = impl_->test_cb_check_anim;
	ImGui::Checkbox("Test checkbox", &sb_data.value);
	ImGui::SameLine( );
	gui::widgets::checkbox2(cb_data, sb_data.value, true, std::addressof(cb_data), std::addressof(cb_check_anim));

	auto& sdata = impl_->slider_data;
	gui::widgets::slider(impl_->slider_text, sdata, &impl_->slider_bg_anim, &impl_->slider_anim);
	ImGui::SliderFloat("imgui slider", &sdata.value, sdata.min, sdata.max, "%.1f", ImGuiSliderFlags_NoRoundToFormat);
}

CHEAT_REGISTER_SERVICE(aimbot);
