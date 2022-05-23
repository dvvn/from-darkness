module;

#include <cheat/core/object.h>

module cheat.hooks.initializer;
import cheat.hooks.loader;

import cheat.hooks.vgui_surface.lock_cursor;
import cheat.hooks.studio_render.draw_model;
import cheat.hooks.client_mode.create_move;
import cheat.hooks.client.frame_stage_notify;
import cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.hooks.c_base_animating.should_skip_animation_frame;
import cheat.hooks.c_base_animating.standard_blending_rules;
import cheat.hooks.c_base_entity.estimate_abs_velocity;

struct initializer_csgo final : hooks_initializer
{
    void operator()() override
    {
        using namespace cheat::hooks;
        loader->add<vgui_surface::lock_cursor>();
        loader->add<studio_render::draw_model>();
        loader->add<client_mode::create_move>();
        loader->add<client::frame_stage_notify>();
        loader->add<c_csplayer::do_extra_bone_processing>();
        loader->add<c_base_animating::should_skip_animation_frame>();
        loader->add<c_base_animating::standard_blending_rules>();
        loader->add<c_base_entity::estimate_abs_velocity>();
    }
};

CHEAT_OBJECT_BIND(hooks_initializer, initializer_csgo, _Csgo_hooks_init)
