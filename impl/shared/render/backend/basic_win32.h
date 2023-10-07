#pragma once

#include "diagnostics/fatal.h"
#include "functional/overload.h"

#include <Windows.h>

#include <utility>

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

struct win32_backend_info final
{
    HWND id;

    WNDPROC proc() const;
    win32_window_size size() const;
    bool minimized() const;
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

enum class win32_backend_update_response : uint8_t
{
    /**
     * \brief system backend does nothing
     */
    skipped = 1 << 0,
    /**
     * \brief system backend does something
     */
    updated = 1 << 1,
    /**
     * \copydoc updated, message processing blocked
     */
    locked  = 1 << 2,
};

template <win32_backend_update_response Response, typename Fn>
struct win32_backend_update_callback final : overload_t<Fn>
{
    constexpr static win32_backend_update_response value = Response;
    using overload_t<Fn>::operator();
    using overload_t<Fn>::overload_t;
};

struct win32_backend_update_unchanged
{
    LRESULT operator()(LRESULT const original) const
    {
        return original;
    }
};

struct win32_backend_update_override
{
    LRESULT value;

    LRESULT operator()() const
    {
        return value;
    }
};

template <win32_backend_update_response Response, typename Fn>
constexpr auto make_win32_backend_update_response(Fn fn) -> win32_backend_update_callback<Response, Fn>
{
    return win32_backend_update_callback<Response, Fn>{std::move(fn)};
}

template <win32_backend_update_response Response>
constexpr auto make_win32_backend_update_response(win32_backend_update_unchanged) -> win32_backend_update_callback<Response, win32_backend_update_unchanged>
{
    return {};
}

class win32_backend_update_finish
{
    using raw_response = std::underlying_type_t<win32_backend_update_response>;
    using response     = win32_backend_update_response;

    template <class... Args>
    constexpr static bool know_response(response const current_response)
    {
        return ((std::to_underlying(current_response) | std::to_underlying(Args::value)) || ...);
    }

    constexpr static bool know_response(response const left, response const right)
    {
        return std::to_underlying(left) | std::to_underlying(right);
    }

    response response_;

    HWND window_;
    UINT message_;
    WPARAM wparam_;
    LPARAM lparam_;

    LRESULT original_;

    template <response CurrentResponse, class Fn, class... Next>
    LRESULT select_callback(Fn& callback, Next&... next_callback) const
    {
        if constexpr (know_response<Fn>(CurrentResponse))
        {
            if constexpr (std::invocable<Fn, HWND, UINT, WPARAM, LPARAM, LRESULT>)
                return callback(window_, message_, wparam_, lparam_, original_);
            else if constexpr (std::invocable<Fn, HWND, UINT, WPARAM, LPARAM>)
                return callback(window_, message_, wparam_, lparam_);
            else if constexpr (std::invocable<Fn, LRESULT>)
                return callback(original_);
            else if constexpr (std::invocable<Fn>)
                return callback();
            else
                unreachable();
        }
        else if constexpr (sizeof...(Next) != 0)
            return select_callback<CurrentResponse>(next_callback...);
        else
            unreachable();
    }

  public:
    win32_backend_update_finish(
        response const response,                                                   //
        HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam, //
        LRESULT const return_value)
        : response_(response)
        , window_(window)
        , message_(message)
        , wparam_(wparam)
        , lparam_(lparam)
        , original_(return_value)
    {
    }

    template <class... Args>
    LRESULT operator()(Args... callback) const
    {
#define CHECK_VALUE(_V_)                       \
    if constexpr (know_response<Args...>(_V_)) \
        if (know_response(response_, _V_))     \
            return select_callback<_V_>(callback...);

        CHECK_VALUE(response::skipped);
        CHECK_VALUE(response::updated);
        CHECK_VALUE(response::locked);

#undef CHECK_VALUE

        unreachable();
    }
};

class basic_win32_backend
{
  protected:
    ~basic_win32_backend();

    basic_win32_backend(HWND window);

  public:
    void new_frame();

    win32_backend_update_finish update(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
};
} // namespace fd