#pragma once

#include <fd/assert_data.h>

namespace fd
{
struct basic_assert_handler
{
    virtual ~basic_assert_handler() = default;

    virtual void run(const assert_data& adata) const noexcept       = 0;
    virtual void run_panic(const assert_data& adata) const noexcept = 0;
};

void set_assert_handler(basic_assert_handler* handler);
} // namespace fd