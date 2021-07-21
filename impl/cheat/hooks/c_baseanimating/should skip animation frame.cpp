#include "should skip animation frame.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/utils/signature.h"

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;
using namespace utl;
using namespace csgo;

should_skip_animation_frame::should_skip_animation_frame( )
{
	this->Wait_for<netvars>( );
}

void should_skip_animation_frame::Load( )
{
#ifndef CHEAT_GUI_TEST
	this->target_func_ = method_info::make_custom(false, []
	{
		cheat::detail::csgo_interface_base ifc;
		ifc.from_sig("client.dll", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02", 0, 0);
		return ifc.addr( ).cast<LPVOID>( );
	});

	this->hook( );
	this->enable( );
#endif
}

void should_skip_animation_frame::Callback(/*float current_time*/)
{
	if (override_return__)
		this->return_value_.store_value(override_return_to__);
	else
	{
		const auto pl = this->Target_instance( );
		const auto client_class = pl->GetClientClass( );
		if (client_class->ClassID != ClassId::CCSPlayer)
			return;

		const auto animate_this_frame = pl->m_bClientSideAnimation( );
		const auto skip_this_frame = animate_this_frame == false;
		this->return_value_.store_value(skip_this_frame);

		(void)client_class;
	}
}

void should_skip_animation_frame::render( )
{
	ImGui::Checkbox("override return", &override_return__);
	if (override_return__)
	{
		const auto pop = memory_backup(ImGui::GetStyle( ).ItemSpacing.x, 0);
		(void)pop;

		ImGui::SameLine( );
		ImGui::Text(" to ");
		ImGui::SameLine( );
		if (ImGui::RadioButton("false ", override_return_to__ == false))
			override_return_to__ = false;
		ImGui::SameLine( );
		if (ImGui::RadioButton("true", override_return_to__ == true))
			override_return_to__ = true;
	}
}
