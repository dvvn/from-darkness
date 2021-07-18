#include "selectable base.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;
using namespace utl;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * anim__.value( )));

selectable_base::selectable_base(bool selected)
{
	if (selected)
		anim__.set(1);
	else
	{
		anim__.set(-1);
		anim__.finish( );
	}
}

void selectable_base::select( )
{
	anim__.set(1);
}

void selectable_base::deselect( )
{
	anim__.set(-1);
}

void selectable_base::toggle( )
{
	anim__.set(selected( ) ? -1 : 1);
}

bool selectable_base::selected( ) const
{
	return anim__.dir( ) == 1;
}

bool selectable_base::animating( ) const
{
	return anim__.updating( );
}

bool selectable_base::Update( )
{
	return anim__.update( );
}

float selectable_base::Anim_value( ) const
{
	return anim__.value( );
}


