#pragma once

#include "diagnostics/fatal.h"
#include "internal/winapi.h"
#include "render/basic_backend.h"

#include <concepts>

namespace fd
{
struct basic_win32_backend : basic_backend
{
    enum response_type : uint8_t
    {
        /**
         * \brief system backend does nothing
         */
        idle,
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
        update_result(LRESULT value, response_type result)
            : value_(value)
            , result_(result)
        {
        }

        template <typename Fn>
        LRESULT finish(Fn &&on_non_lock, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const
        {
            if constexpr (!std::invocable<Fn, HWND, UINT, WPARAM, LPARAM>)
                return value_;
            else
                switch (result_)
                {
                case idle:
                case updated:
                    return on_non_lock(window, message, wparam, lparam);
                case locked:
                    return value_;
                default:
                    unreachable();
                }
        }

        template <typename Idle, typename Upd>
        LRESULT finish(Idle &&on_idle, Upd &&on_update, HWND window, UINT message, WPARAM wparam, LPARAM lparam) const
        {
            switch (result_)
            {
            case idle:
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

  protected:
    basic_win32_backend(HWND window);

  public:
    void destroy() override;
    void new_frame() override;
    virtual update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

    virtual WNDPROC proc() const = 0;
    virtual HWND id() const      = 0;
};
} // namespace fd