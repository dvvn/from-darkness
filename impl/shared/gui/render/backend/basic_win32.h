#pragma once

#include <Windows.h>

#include <cstdint>

namespace fd::gui
{
struct win32_window_size_simple
{
    LONG w;
    LONG h;

    win32_window_size_simple();
    win32_window_size_simple(RECT const& rect);
    win32_window_size_simple(LONG w, LONG h);

    bool operator==(win32_window_size_simple const& other) const;
};

struct win32_window_size : win32_window_size_simple
{
    LONG x;
    LONG y;

    win32_window_size();
    win32_window_size(RECT const& rect);

    win32_window_size& operator=(win32_window_size_simple const& parent_size);
};

struct win32_window_info final
{
    HWND id;

    WNDPROC proc() const;
    win32_window_size size() const;
    bool minimized() const;
};

struct win32_window_info_static
{
    union
    {
        HWND id;
        win32_window_info dynamic;
    };

    WNDPROC proc;
    win32_window_size size;
    bool minimized;

    win32_window_info_static(HWND id);
    win32_window_info_static(win32_window_info info);
};

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