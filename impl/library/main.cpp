#include "cheat/hooks/base_includes.h"
#include "cheat/netvars/includes.h"
#include "cheat/players/player_includes.h"

#include <nstd/runtime_assert.h>
#include <Windows.h>
#include <d3d9.h>

import cheat.console;
import cheat.csgo.awaiter;
import cheat.csgo.interfaces;
import cheat.gui;
import cheat.netvars;
import cheat.players;
import cheat.hooks.winapi;
import cheat.hooks.vgui_surface;
import cheat.hooks.studio_render;
import cheat.hooks.imgui;
import cheat.hooks.directx;
import cheat.hooks.client_mode;
import cheat.hooks.client;
import cheat.hooks.c_csplayer;
import cheat.hooks.c_base_animating;
import cheat.hooks.c_base_entity;

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	using namespace cheat;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		service_deps_getter_add_allow_skip = false;

		auto& loader = services_loader::get( );
		loader.module_handle = hModule;

		auto deps = loader.deps( );
		//deps.add<>();
		deps.add<csgo_awaiter>( );
		deps.add<console>( );
		deps.add<csgo_interfaces>( );
		deps.add<gui::context>( );
		deps.add<gui::menu>( );
		deps.add<netvars>( );
		deps.add<players_list>( );
		deps.add<hooks::winapi::wndproc>( );
		deps.add<hooks::vgui_surface::lock_cursor>( );
		deps.add<hooks::studio_render::draw_model>( );
		deps.add<hooks::imgui::PushClipRect>( );
		deps.add<hooks::directx::present>( );
		deps.add<hooks::directx::reset>( );
		deps.add<hooks::client_mode::create_move>( );
		deps.add<hooks::client::frame_stage_notify>( );
		deps.add<hooks::c_csplayer::do_extra_bone_processing>( );
		deps.add<hooks::c_base_animating::should_skip_animation_frame>( );
		deps.add<hooks::c_base_animating::standard_blending_rules>( );
		deps.add<hooks::c_base_entity::estimate_abs_velocity>( );

		loader.start_async(services_loader::async_detach( ));
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		services_loader::get( ).reset(true);
		break;
	}
	}

	return TRUE;
}


