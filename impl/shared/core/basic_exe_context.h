#pragma once
#include "core/basic_context.h"

namespace fd
{
namespace detail
{
inline int exe_main(int const argc, int* argv) noexcept
{
    std::ignore = argc;
    std::ignore = argv;

    return attach_context() ? EXIT_SUCCESS : EXIT_FAILURE;
}
} // namespace detail
} // namespace fd

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main(int const argc, int* argv)
{
    return fd::detail::exe_main(argc, argv);
}