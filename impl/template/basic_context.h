#pragma once

#include "logger.h"
#include "noncopyable.h"
#include "system_console.h"

namespace fd
{
namespace detail
{
class basic_context : public noncopyable
{
  public:
    static constexpr auto make_logger = []() /*[[nodiscard]]*/-> basic_logger<system_console> {
        return {};
    };

#ifdef _DEBUG
    static constexpr auto make_debug_logger = make_logger;
#else
    [[nodiscard]]
    static empty_logger make_debug_logger()
    {
        return {};
    }
#endif
};
} // namespace detail
} // namespace fd