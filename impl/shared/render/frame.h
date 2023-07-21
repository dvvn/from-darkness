#pragma once

#include "basic_frame.h"
#include "object_holder.h"
#include "preprocessor.h"

namespace fd
{
struct basic_render_backend;
struct basic_system_backend;
struct basic_render_context;

struct basic_menu;
struct basic_variables_group;

#define RENDER_FRAME_CONSTRUCT_ARGS                                 \
    FD_GROUP_ARGS(basic_render_backend *, basic_system_backend *, ) \
    FD_GROUP_ARGS(basic_render_context *, )                         \
    FD_GROUP_ARGS(basic_menu *, basic_variables_group **, size_t)

class render_frame_simple;
struct render_frame_full;

FD_OBJECT_FWD(render_frame_simple, basic_render_frame, RENDER_FRAME_CONSTRUCT_ARGS);
FD_OBJECT_FWD(render_frame_full, basic_render_frame, RENDER_FRAME_CONSTRUCT_ARGS);

#undef RENDER_FRAME_CONSTRUCT_ARGS

} // namespace fd