#include "menu.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"
#include "cheat/gui/_imgui extension/push style var.h"

using namespace cheat::gui;
using namespace cheat::utl;
using namespace menu;

menu_obj::menu_obj( )
{
	this->Wait_for<settings>( );

	constexpr uint32_t compile_year = (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
	constexpr uint32_t compile_month = []
	{
		switch (__DATE__[0])
		{
			case 'J': // Jan, Jun or Jul
				return (__DATE__[1] == 'a' ? 1 : __DATE__[2] == 'n' ? 6 : 7);
			case 'F': // Feb
				return 2;
			case 'M': // Mar or May
				return (__DATE__[2] == 'r' ? 3 : 5);
			case 'A': // Apr or Aug
				return (__DATE__[2] == 'p' ? 4 : 8);
			case 'S': // Sep
				return 9;
			case 'O': // Oct
				return 10;
			case 'N': // Nov
				return 11;
			case 'D': // Dec
				return 12;
		}
	}( );

	// ReSharper disable once CppUnreachableCode
	constexpr uint32_t compile_day = __DATE__[4] == ' ' ? __DATE__[5] - '0' : (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

	constexpr string::value_type iso_date[] =
	{
		compile_year / 1000 + '0', compile_year % 1000 / 100 + '0', compile_year % 100 / 10 + '0', compile_year % 10 + '0',
		'.',
		compile_month / 10 + '0', compile_month % 10 + '0',
		'.',
		compile_day / 10 + '0', compile_day % 10 + '0',
		'\0'
	}; 

	string name = CHEAT_NAME;
	name += " | ";
	name += iso_date;
#ifdef _DEBUG
	name += _CONCAT(" | ", __TIME__);
#endif

#ifdef CHEAT_GUI_TEST
	name += " (gui test)";
#endif

#ifdef _DEBUG
	name += " DEBUG MODE";
#endif

	menu_title__ = move(name);
}

void menu_obj::render(float bg_alpha)
{
#if defined(_DEBUG) ||  defined(CHEAT_TEST_EXE)
	ImGui::ShowDemoWindow( );
#endif

	auto& style = ImGui::GetStyle( );

	memory_backup<float> alpha_backup;
	(void)alpha_backup;

	if (bg_alpha != style.Alpha)
		alpha_backup = memory_backup(style.Alpha, bg_alpha);

	constexpr auto dummy_text = string_view("W");
	const auto     sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
	const auto     min_size = ImGui::GetFontSize( ) + //small button size
							  style.ItemInnerSpacing.x +
							  sample_size.x * menu_title__.raw( ).size( ) + //string size
							  style.FramePadding.x * 2.f +                  //space between and after
							  style.WindowBorderSize;
	//ImGui::SetNextWindowContentSize({min_size, 0});

	memory_backup<float> min_size_backup;
	(void)min_size_backup;

	if (min_size > style.WindowMinSize.x)
		min_size_backup = memory_backup(style.WindowMinSize.x, min_size);

	if (ImGui::Begin(menu_title__, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		min_size_backup.reset( );
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
