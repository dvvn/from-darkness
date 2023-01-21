#pragma once

#include <fd/assert_handler.h>
#include <fd/string.h>

#include <mutex>

namespace fd
{
    // ReSharper disable once CppInconsistentNaming
    void _default_assert_handler(const assert_data& adata, bool interrupt);

    template <typename Callback>
    class default_assert_handler final : public basic_assert_handler
    {
        mutable std::mutex mtx_;
        Callback           callback_;

      public:
        default_assert_handler(Callback callback)
            : callback_(std::move(callback))
        {
            basic_assert_handler::set(this);
        }

        void run(const assert_data& adata) const noexcept override
        {
            const std::lock_guard guard(mtx_);
            callback_(adata);
            _default_assert_handler(adata, false);
        }

        void run_panic(const assert_data& adata) const noexcept override
        {
            const std::lock_guard guard(mtx_);
            callback_(adata);
            _default_assert_handler(adata, true);
        }
    };

    template <typename Callback>
    default_assert_handler(Callback) -> default_assert_handler<std::decay_t<Callback>>;

    wstring parse(const assert_data& adata);
} // namespace fd