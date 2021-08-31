#include "group.h"

#include <imgui.h>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

group::group( )
{
	if (fade_.time( ) == fade_.default_time)
		fade_.set_time(animator::default_time - animator::default_time * 0.3f); //30% faster
	this->show( );
	fade_.finish( );
}

void group::show( )
{
	fade_.set(1);
}

void group::begin( )
{
	this->Animate( );
	ImGui::BeginGroup( );
}

void group::end( )
{
	ImGui::EndGroup( );
	fade_alpha_backup_.restore( );
}
