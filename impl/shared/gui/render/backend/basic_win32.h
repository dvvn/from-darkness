#pragma once

#include <Windows.h>

#include <cstdint>

namespace fd::gui
{
enum class win32_backend_update_response : uint8_t
{
    /**
     * \brief system backend does nothing
     */
    skipped,
    /**
     * \brief system backend does something
     */
    updated,
    /**
     * \copydoc updated, message processing blocked
     */
    locked,
};

struct win32_backend_update_result
{
    win32_backend_update_response response;
    LRESULT retval;
};

class basic_win32_backend
{
  protected:
    ~basic_win32_backend();
    basic_win32_backend(HWND window);

  public:
    static void new_frame();

    static win32_backend_update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
};
} // namespace fd::gui