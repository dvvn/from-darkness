#pragma once
#include "functional/ignore.h"

namespace fd
{
namespace detail
{
class exe_context
{
  public:
    static bool run();

    static bool start(int argc, int* argv)
    {
        ignore_unused(argc, argv);
        return run();
    }
};

inline exe_context exe_context_instance;
} // namespace detail

using context = detail::exe_context;
} // namespace fd

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main(int argc, int* argv)
{
    using fd::detail::exe_context_instance;

    return exe_context_instance.start(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}