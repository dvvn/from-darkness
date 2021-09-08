#include "aimbot.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "cheat/gui/tools/string wrapper.h"

#include <nstd/type name.h>

#include <imgui.h>

using namespace cheat::features;

aimbot::aimbot( )
	: non_abstract_label((nstd::drop_namespaces(nstd::type_name<aimbot>)))
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

void aimbot::render( )
{
	ImGui::Text("1");
	ImGui::SameLine( );
	ImGui::Text("1.5");
	ImGui::Text("2");
}
