#pragma once

#include "runtime_error.h"
#include "internal/winapi.h"

namespace fd
{
class system_error : public runtime_error
{
    using code_type = HRESULT;

    HRESULT code_;

  public:
    system_error(HRESULT code, char const *message = default_exception_message) noexcept;
    system_error(DWORD code, char const *message = default_exception_message) noexcept;
    system_error(char const *message = default_exception_message) noexcept;

    code_type code() const noexcept;
    char const *error() const noexcept;
};
} // namespace fd