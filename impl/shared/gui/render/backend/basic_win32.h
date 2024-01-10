#pragma once

#include "noncopyable.h"

#include <Windows.h>

namespace fd
{
namespace gui
{
namespace detail
{
class win32_backend_update_result
{
  public:
    using size_type = int;

  private:
    size_type events_before_;
    LRESULT retval_;
    size_type events_after_;

  public:
    win32_backend_update_result(size_type events_before, LRESULT retval, size_type events_after);

    LRESULT retval_or_default(HWND window, UINT message, WPARAM wparam, LPARAM lparam) const;

    template <typename Fn>
    LRESULT retval_or_default_or_original(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam, Fn& original) const
    {
        LRESULT retval;
        if (locked())
            retval = retval_;
        else if (updated())
            retval = DefWindowProc(window, message, wparam, lparam);
        else
            retval = original(window, message, wparam, lparam);
        return retval;
    }

    LRESULT retval_or_default_or_original(HWND window, UINT message, WPARAM wparam, LPARAM lparam, WNDPROC original) const;

    // message processing blocked
    bool locked() const;
    // backend does something
    bool updated() const;

    LRESULT get() const;
};
} // namespace detail

class basic_win32_backend : public noncopyable
{
  public:
    using update_result = detail::win32_backend_update_result;

  protected:
    ~basic_win32_backend();
    basic_win32_backend(HWND window);

  public:
    static void new_frame();

    static update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT update_simple(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
};
} // namespace gui
} // namespace fd