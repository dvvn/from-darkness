#pragma once

#include <fd/assert_handler.h>
#include <fd/string.h>

#include <mutex>
#include <vector>

namespace fd
{
    class default_assert_handler final : public basic_assert_handler
    {
        using function_type = function<void(const assert_data&) const>;

        std::vector<function_type> data_;
        mutable std::mutex mtx_;

      public:
        default_assert_handler();
        
        void add(function_type fn);
        void operator()(const assert_data& adata) const noexcept override;
    };

    wstring parse_assert_data(const assert_data& adata);
} // namespace fd
