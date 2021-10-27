#include "draw_model.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::studio_render;

void draw_model::callback(DrawModelResults_t* results, const DrawModelInfo_t& info,
						  matrix3x4_t* bone_to_world,
						  float* flex_weights, float* flex_delayed_weights,
						  const Vector& model_origin, DrawModelFlags_t flags)
{
}
