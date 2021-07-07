#include "anti aim.h"

using namespace cheat::features;

anti_aim::anti_aim( ) : settings_data("anti aim")
{
}

auto anti_aim::render( ) -> void
{
	ImGui::Checkbox("test2", &test__);
	ImGui::Text("test3");
	ImGui::Text("test4");
}

auto anti_aim::update( ) -> void
{
	Load_or_create("test2", test__);
}
