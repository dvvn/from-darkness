#pragma once
#include "noncopyable.h"
#include "render/backend/basic_win32.h"

namespace fd
{
class win32_backend_native final : public basic_win32_backend, public noncopyable
{
#ifdef _DEBUG
    HWND window_;

    win32_backend_native(HWND window);
#endif

  public:
    ~win32_backend_native();
    win32_backend_native();

    void destroy() override;

#ifdef _DEBUG
    update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) override;
#endif
};

} // namespace fd