#include "aimbot.h"

using namespace cheat::features;

aimbot::aimbot( ) : settings_data("aimbot") // settings_data("rage.aimbot")//ideal result
{
}

void aimbot::render( )
{
	ImGui::Checkbox("test", &test__);
}

void aimbot::update( )
{
	//Load_or_create("test", test__);
}
