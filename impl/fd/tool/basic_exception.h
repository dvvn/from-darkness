#pragma once
#include <cstdint>

namespace fd
{
struct string_view;

struct exception
{
    virtual ~exception()                = default;
    virtual string_view message() const = 0;
};

struct basic_runtime_error : exception
{
};

class basic_system_error : public basic_runtime_error
{
    uintptr_t code_;

  protected:
    size_t format(char **buffer, size_t buffer_length) const noexcept;

  public:
    basic_system_error(uintptr_t code)noexcept;
    basic_system_error() noexcept;
    uintptr_t code() const noexcept;
    virtual string_view error() const noexcept = 0;
};

}