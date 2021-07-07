#include "menu.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"
#include "cheat/gui/imgui/push style var.h"

using namespace cheat::gui;
using namespace menu;

menu_obj::menu_obj( )
{
	this->Wait_for<settings>( );
}

void menu_obj::render(float bg_alpha)
{
#if defined(_DEBUG) ||  defined(CHEAT_TEST_EXE)
	ImGui::ShowDemoWindow( );
#endif

	auto pop = imgui::push_style_var(ImGuiStyleVar_Alpha, bg_alpha);
	(void)pop;

	if (ImGui::Begin("test", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		renderer__.render( );
	}
	ImGui::End( );
}

bool menu_obj::visible( ) const
{
	return visible__ || animating( );
}

bool menu_obj::animating( ) const
{
	return fade__.updating( );
}

bool menu_obj::active( ) const
{
	return visible__ && fade__.done(1);
}

bool menu_obj::toggle(UINT msg, WPARAM wparam)
{
	if (wparam != hotkey__)
		return false;

	if (msg == WM_KEYDOWN)
	{
		//hide press from app
		return true;
	}
	if (msg == WM_KEYUP)
	{
		visible__ = !visible__;
		fade__.set(visible__ ? 1 : -1);
		return true;
	}

	return false;
}

void menu_obj::toggle( )
{
	toggle(WM_KEYUP, hotkey__);
}

bool menu_obj::animate( )
{
	return fade__.update( );
}

float menu_obj::get_fade( ) const
{
	return fade__.value( );
}

using namespace cheat::utl;

void menu_obj::Load( )
{
	auto  rage_abstract = abstract_page( );
	auto& rage = *rage_abstract.init<page_with_tabs>("rage");

	rage.add_page(features::aimbot::get_ptr( ));
	rage.add_page(features::anti_aim::get_ptr( ));

	renderer__.add_page(move(rage_abstract));
	renderer__.add_page({"settings", settings::get_ptr( )});

	renderer__.init( );
}
