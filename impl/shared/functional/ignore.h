#pragma once

namespace fd
{
inline constexpr auto ignore_unused = [](auto&&...) -> void {
    //
};
} // namespace fd