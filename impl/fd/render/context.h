#pragma once

#include <cstdint>

namespace fd
{
void *create_render_context(void *window, void *backend) noexcept;
void destroy_render_context() noexcept;
void render_backend_detach();

enum class render_message_result : uint8_t
{
    idle,
    updated,
    locked
};

void process_render_message(
    void *window,
    size_t message,
    size_t wParam,
    size_t lParam,
    render_message_result *result = nullptr);

void reset_render_context();

bool begin_render_frame();
void end_render_frame();

class render_frame
{
    bool valid_;

  public:
    render_frame();
    ~render_frame();

    render_frame(render_frame const &other)            = delete;
    render_frame &operator=(render_frame const &other) = delete;

    explicit operator bool() const;
};
}