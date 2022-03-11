module;

#include <functional>

module cheat.hooks:initializer;
import :loader;

import cheat.hooks.winapi;
import cheat.hooks.imgui;
import cheat.hooks.directx;

import cheat.hooks.vgui_surface;
import cheat.hooks.studio_render;
import cheat.hooks.client_mode;
import cheat.hooks.client;
import cheat.hooks.c_csplayer;
import cheat.hooks.c_base_animating;
import cheat.hooks.c_base_entity;

using namespace cheat;

void hooks::init_basic( )
{
	using namespace hooks;
	add<winapi::wndproc>( );
	add<imgui::PushClipRect>( );
	add<directx::reset>( );
	add<directx::present>( );
}

void hooks::init_all( )
{
	init_basic( );
	using namespace hooks;
	add<vgui_surface::lock_cursor>( );
	add<studio_render::draw_model>( );
	add<client_mode::create_move>( );
	add<client::frame_stage_notify>( );
	add<c_csplayer::do_extra_bone_processing>( );
	add<c_base_animating::should_skip_animation_frame>( );
	add<c_base_animating::standard_blending_rules>( );
	add<c_base_entity::estimate_abs_velocity>( );
	//add<other::rta_holder>( );
}