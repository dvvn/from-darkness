#include "aimbot.h"

using namespace cheat::features;

aimbot::aimbot( ) : settings_data("aimbot") // settings_data("rage.aimbot")//ideal result
{
}

auto aimbot::render( ) -> void
{
	ImGui::Checkbox("test", &test__);
}

auto aimbot::update( ) -> void
{
	Load_or_create("test", test__);
}
