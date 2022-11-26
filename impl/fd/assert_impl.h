#pragma once

#include <fd/assert_base.h>
#include <fd/string.h>

#include <mutex>
#include <vector>

namespace fd
{
    class default_assert_handler : public basic_assert_handler
    {
        std::vector<function_view<void(const assert_data&) const>> data_;
        mutable std::mutex mtx_;

      public:
        default_assert_handler();

        template <typename Fn>
        void add(Fn&& fn)
        {
            const std::lock_guard guard(mtx_);
            data_.emplace_back(std::forward<Fn>(fn));
        }

        void operator()(const assert_data& adata) const noexcept override;
    };

    struct parse_assert_data_impl
    {
        wstring operator()(const assert_data& adata) const;
    };

    constexpr parse_assert_data_impl parse_assert_data;
} // namespace fd
