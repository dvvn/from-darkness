#pragma once
#include "basic_context.h"

namespace fd
{
namespace detail
{
class exe_context_holder : public basic_context
{
  protected:
    bool attach();
};

inline struct : exe_context_holder
{
    int operator()(int const argc, int* argv)
    {
        std::ignore = argc;
        std::ignore = argv;

        return attach() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
} exe_context;
} // namespace detail

using context = detail::exe_context_holder;

bool context_holder(context* ctx);

inline bool context::attach()
{
    return context_holder(this);
}
} // namespace fd

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main(int const argc, int* argv)
{
    return fd::detail::exe_context(argc, argv);
}