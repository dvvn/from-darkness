﻿module;

#include <fd/object.h>

module fd.GlowEffect;
import fd.rt_modules;

using namespace fd;

FD_OBJECT_ATTACH_EX(CGlowObjectManager*, fd::rt_modules::client_fn().find_interface_sig<"0F 11 05 ? ? ? ? 83 C8 01">().plus(3).deref<1>());

void GlowObject_t::Set(const math::color& glow_color, GlowRenderStyles style)
{
    this->color                  = { static_cast<float>(glow_color.r), static_cast<float>(glow_color.g), static_cast<float>(glow_color.b), static_cast<float>(glow_color.a) };
    this->bloom_amount           = 1.0f;
    this->render_when_occluded   = true;
    this->render_when_unoccluded = false;
    this->render_style           = style;
}

bool GlowObject_t::IsEmpty() const
{
    return next_free_slot != GLOW_ENTRY_IN_USE;
}
