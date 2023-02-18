#pragma once

#include <fd/assert_handler.h>
#include <fd/string.h>

#include <mutex>

namespace fd
{
struct _default_assert_handler : basic_assert_handler
{
    void run(const assert_data& adata) const noexcept override;
    void run_panic(const assert_data& adata) const noexcept override;
};

template <typename Callback>
class default_assert_handler final : public _default_assert_handler
{
    using mutex      = std::mutex;
    using lock_guard = std::lock_guard<mutex>;

    mutable mutex mtx_;
    Callback      callback_;

  public:
    default_assert_handler(Callback callback)
        : callback_(std::move(callback))
    {
        set_assert_handler(this);
    }

    void run(const assert_data& adata) const noexcept override
    {
        const lock_guard guard(mtx_);
        callback_(adata);
        _default_assert_handler::run(adata);
    }

    void run_panic(const assert_data& adata) const noexcept override
    {
        const lock_guard guard(mtx_);
        callback_(adata);
        _default_assert_handler::run_panic(adata);
    }
};

template <typename Callback>
default_assert_handler(Callback) -> default_assert_handler<std::decay_t<Callback>>;

wstring parse(const assert_data& adata);
} // namespace fd