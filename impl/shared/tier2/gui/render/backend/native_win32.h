#pragma once
#include "tier1/noncopyable.h"
#include "tier2/gui/render/backend/basic_win32.h"

namespace FD_TIER2(gui)
{
class basic_native_win32_backend : public basic_win32_backend
{
    class main_window_finder;
    static HWND find_main_window() noexcept;

    HWND window_;

  protected:
    basic_native_win32_backend();
    basic_native_win32_backend(HWND window);

  public:
    HWND window() const;
};

template <class Context>
struct native_win32_backend final : basic_native_win32_backend, noncopyable
{
    native_win32_backend(Context*)
    {
    }

    native_win32_backend(Context*, HWND window)
        : basic_native_win32_backend{window}
    {
    }
};
} // namespace FD_TIER2(gui)