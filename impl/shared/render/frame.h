#pragma once

#include "basic_frame.h"

#include <cstdint>

namespace fd
{

class render_frame_simple : public basic_render_frame
{
    friend class render_frame_full;

    struct basic_render_backend *render_backend_;
    struct basic_system_backend *system_backend_;
    struct basic_render_context *render_context_;

    struct basic_menu *menu_;
    struct basic_variables_group **menu_data_;
    uint8_t menu_data_length_;

  public:
    render_frame_simple(
        basic_render_backend *render_backend,
        basic_system_backend *system_backend,
        basic_render_context *render_context,
        basic_menu *menu,
        basic_variables_group **menu_data,
        uint8_t menu_data_length
    );

    void render() override;
};

class render_frame_full : public render_frame_simple
{
  public:
    using render_frame_simple::render_frame_simple;

    void render() override;
};

} // namespace fd