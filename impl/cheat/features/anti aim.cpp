#include "anti aim.h"

using namespace cheat::features;

anti_aim::anti_aim( ) : settings_data("anti aim")
{
}

void anti_aim::render( )
{
	ImGui::Checkbox("test2", &test__);
	ImGui::Text("test3");
	ImGui::Text("test4");
}

void anti_aim::update( )
{
	//Load_or_create("test2", test__);
}
