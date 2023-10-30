#pragma once

#include <Windows.h>

#include <functional>
#include <utility>

namespace fd
{
struct simple_win32_window_size
{
    LONG w;
    LONG h;

    simple_win32_window_size();
    simple_win32_window_size(RECT const& rect);
    simple_win32_window_size(LONG w, LONG h);

    bool operator==(simple_win32_window_size const& other) const;
};

struct win32_window_size : simple_win32_window_size
{
    LONG x;
    LONG y;

    win32_window_size();
    win32_window_size(RECT const& rect);

    win32_window_size& operator=(simple_win32_window_size const& parent_size);
};

struct win32_window_info final
{
    HWND id;

    WNDPROC proc() const;
    win32_window_size size() const;
    bool minimized() const;
};

struct static_win32_window_info
{
    union
    {
        HWND id;
        win32_window_info dynamic;
    };

    WNDPROC proc;
    win32_window_size size;
    bool minimized;

    static_win32_window_info(HWND id);
    static_win32_window_info(win32_window_info info);
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

using win32_backend_update_result = std::pair<win32_backend_update_response, LRESULT>;

class basic_win32_backend
{
  protected:
    ~basic_win32_backend();

    basic_win32_backend(HWND window);

  public:
    static void new_frame();

    static win32_backend_update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
};
} // namespace fd