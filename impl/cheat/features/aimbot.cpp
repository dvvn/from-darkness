#include "aimbot.h"

#include "cheat/gui/tools/string wrapper.h"

#include <nstd/type name.h>

#include <imgui.h>

using namespace cheat::features;

aimbot::aimbot( )
	: non_abstract_label(gui::tools::string_wrapper(std::string(nstd::type_name<aimbot, "cheat::features">)))
{
}

aimbot::~aimbot( )
{
}

void aimbot::save(json& in) const
{
}

void aimbot::load(const json& out)
{
}

std::wstring_view aimbot::title( ) const
{
	return label( );
}

void aimbot::render( )
{
	ImGui::Text("1");
	ImGui::SameLine( );
	ImGui::Text("1.5");
	ImGui::Text("2");
}
