#pragma once

#include "context.h"

namespace fd
{
enum class process_message_result : uint8_t
{
    idle,
    updated,
    locked
};

void reset(render_context_ptr ctx);
void resize(render_context_ptr ctx, UINT w, UINT h);
void process_message(
    render_context_ptr ctx,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    process_message_result *result = nullptr);
}