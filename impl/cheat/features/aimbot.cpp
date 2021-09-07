#include "aimbot.h"

#include <nstd/type name.h>

using namespace cheat::features;

aimbot::aimbot( )
	: non_abstract_label(std::string(nstd::type_name<aimbot, "cheat::features">))
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
	ImGui::Text(label( ));
}
