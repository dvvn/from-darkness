#include <nstd/runtime_assert.h>

#include <Windows.h>
#include <d3d9.h>

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
//import cheat.hooks.other;

import cheat.hooks.loader;

static DWORD WINAPI setup_hooks(LPVOID hModule)
{
	using namespace cheat;
	using namespace hooks;
	add<winapi::wndproc>( );
	add<vgui_surface::lock_cursor>( );
	add<studio_render::draw_model>( );
	add<imgui::PushClipRect>( );
	add<directx::present>( );
	add<directx::reset>( );
	add<client_mode::create_move>( );
	add<client::frame_stage_notify>( );
	add<c_csplayer::do_extra_bone_processing>( );
	add<c_base_animating::should_skip_animation_frame>( );
	add<c_base_animating::standard_blending_rules>( );
	add<c_base_entity::estimate_abs_velocity>( );
	//add<other::rta_holder>( );

	if (start( ).get( ))
		return TRUE;

	FreeLibraryAndExitThread(static_cast<HMODULE>(hModule), FALSE);
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		const HANDLE hThread = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			setup_hooks, // Thread start address
			hModule,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id
		if (hThread == NULL)
		{
			// Thread creation failed.
			// More details can be retrieved by calling GetLastError()
			return FALSE;
		}
		CloseHandle(hThread);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		cheat::hooks::stop(true);
		break;
	}
	}

	return TRUE;
}


