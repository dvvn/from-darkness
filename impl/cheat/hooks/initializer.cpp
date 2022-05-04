module;

#include <nstd/runtime_assert_core.h>

#include <functional>

module cheat.hooks.initializer;
import cheat.hooks.loader;

//#define CHEAT_GUI_HAVE_EFFECTS

import cheat.hooks.winapi.wndproc;
#ifdef CHEAT_GUI_HAVE_EFFECTS
import cheat.hooks.imgui.PushClipRect;
#endif
import cheat.hooks.directx.present;
import cheat.hooks.directx.reset;

import cheat.hooks.vgui_surface.lock_cursor;
import cheat.hooks.studio_render.draw_model;
import cheat.hooks.client_mode.create_move;
import cheat.hooks.client.frame_stage_notify;
import cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.hooks.c_base_animating.should_skip_animation_frame;
import cheat.hooks.c_base_animating.standard_blending_rules;
import cheat.hooks.c_base_entity.estimate_abs_velocity;

using namespace cheat;

#define ADD_HOOK(_SOURCE_)\
	add<_SOURCE_>( )

void hooks::init_basic( ) runtime_assert_noexcept
{
	using namespace hooks;
	ADD_HOOK(winapi::wndproc);
#ifdef CHEAT_GUI_HAVE_EFFECTS
	ADD_HOOK(imgui::PushClipRect);
#endif
	ADD_HOOK(directx::reset);
	ADD_HOOK(directx::present);
}

void hooks::init_all( ) runtime_assert_noexcept
{
	init_basic( );
	using namespace hooks;
	ADD_HOOK(vgui_surface::lock_cursor);
	ADD_HOOK(studio_render::draw_model);
	ADD_HOOK(client_mode::create_move);
	ADD_HOOK(client::frame_stage_notify);
	ADD_HOOK(c_csplayer::do_extra_bone_processing);
	ADD_HOOK(c_base_animating::should_skip_animation_frame);
	ADD_HOOK(c_base_animating::standard_blending_rules);
	ADD_HOOK(c_base_entity::estimate_abs_velocity);
	//ADD_HOOK(other::rta_holder);
}