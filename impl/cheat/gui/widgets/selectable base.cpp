#include "selectable base.h"

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;
using namespace utl;

//ImGui::PushStyleColor(color_idx, !anim_updated ? header_color : ImVec4(header_color.x, header_color.y, header_color.z, header_color.w * fade_.value( )));

selectable_base::selectable_base(bool selected)
{
	if (selected)
		fade_.set(1);
	else
	{
		fade_.set(-1);
		fade_.finish( );
	}
}

void selectable_base::select( )
{
	fade_.set(1);
}

void selectable_base::deselect( )
{
	fade_.set(-1);
}

void selectable_base::toggle( )
{
	fade_.set(selected( ) ? -1 : 1);
}

bool selectable_base::selected( ) const
{
	return fade_.dir( ) == 1;
}
