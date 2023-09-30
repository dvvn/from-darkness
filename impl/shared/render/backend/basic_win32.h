#pragma once

#include "diagnostics/fatal.h"
#include "internal/winapi.h"
#include "render/basic_system_backend.h"

#include <concepts>

namespace fd
{
struct simple_win32_window_size
{
    LONG w;
    LONG h;

    simple_win32_window_size();
    simple_win32_window_size(RECT const& rect);
};

struct win32_window_size : simple_win32_window_size
{
    LONG x;
    LONG y;

    win32_window_size();
    win32_window_size(RECT const& rect);

    win32_window_size& operator=(simple_win32_window_size const& parent_size);
};

struct win32_backend_info final : basic_system_backend_info
{
    HWND id;

    WNDPROC proc() const;
    win32_window_size size() const;
    bool minimized() const override;
};

struct static_win32_backend_info
{
    union
    {
        HWND id;
        win32_backend_info dynamic;
    };

    WNDPROC proc;
    win32_window_size size;
    bool minimized;

    static_win32_backend_info(HWND id);
    static_win32_backend_info(win32_backend_info info);
};

struct basic_win32_backend : basic_system_backend
{
    enum response_type : uint8_t
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
        locked
    };

    class update_result
    {
        LRESULT value_;
        response_type result_;

      public:
        update_result(LRESULT const value, response_type const result)
            : value_(value)
            , result_(result)
        {
        }

        template <typename Fn>
        LRESULT finish(Fn&& on_non_lock, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const
        {
            if constexpr (!std::invocable<Fn, HWND, UINT, WPARAM, LPARAM>)
                return value_;
            else
                switch (result_)
                {
                case skipped:
                case updated:
                    return on_non_lock(window, message, wparam, lparam);
                case locked:
                    return value_;
                default:
                    unreachable();
                }
        }

        template <typename Idle, typename Upd>
        LRESULT finish(Idle&& on_idle, Upd&& on_update, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const
        {
            switch (result_)
            {
            case skipped:
                if constexpr (std::invocable<Idle, HWND, UINT, WPARAM, LPARAM>)
                    return on_idle(window, message, wparam, lparam);
                return value_;
            case updated:
                if constexpr (std::invocable<Upd, HWND, UINT, WPARAM, LPARAM>)
                    return on_update(window, message, wparam, lparam);
                return value_;
            case locked:
                return value_;
            default:
                unreachable();
            }
        }
    };

    virtual void setup(HWND window);
    void destroy() override;
    void new_frame() override;
    virtual update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

    virtual void update(win32_backend_info* backend_info) const = 0;
    void update(basic_system_backend_info* backend_info) const override;
};
} // namespace fd