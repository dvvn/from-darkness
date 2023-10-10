#include "hook_error.h"

namespace fd
{
char const *hook_error::what() const
{
    auto const msg = runtime_error::what();
    return msg == default_exception_message ? status() : msg;
}
}