module;

#include <fds/hooks/hook.h>

module fds.hooks.studio_render.draw_model;
import fds.csgo.interfaces.StudioRender;

using namespace fds;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

FDS_HOOK(draw_model, member){draw_model_impl(){this->init({&instance_of<IStudioRender>, 29}, &draw_model_impl::callback);
}

void callback(DrawModelResults_t* results, const DrawModelInfo_t& info, math::matrix3x4* bone_to_world, float* flex_weights, float* flex_delayed_weights, const math::vector3& model_origin, DrawModelFlags_t flags) const
{
    call_original(results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
}
}
;

FDS_HOOK_IMPL(draw_model);
