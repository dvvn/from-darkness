module;

#include <cheat/hooks/hook.h>

module cheat.hooks.studio_render.draw_model;
import cheat.csgo.interfaces.StudioRender;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace studio_render;

CHEAT_HOOK(draw_model, member)
{
    draw_model_impl( )
    {
        this->init({&instance_of<IStudioRender>, 29}, &draw_model_impl::callback);
    }

    void callback(DrawModelResults_t* results, const DrawModelInfo_t& info, math::matrix3x4* bone_to_world, float* flex_weights, float* flex_delayed_weights, const math::vector3& model_origin, DrawModelFlags_t flags) const
    {
        call_original(results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
    }
};

CHEAT_HOOK_IMPL(draw_model);
