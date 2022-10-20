module;

#include <utility>

export module fd.assert.impl;
export import fd.assert;
export import fd.string;
import fd.callback;
import fd.mutex;

namespace fd
{
    export class default_assert_handler : public basic_assert_handler
    {
        callback_simple<const assert_data&> data_;
        mutable mutex mtx_;

      public:
        default_assert_handler();

        template <typename Fn>
        void add(Fn&& fn)
        {
            const lock_guard guard = mtx_;
            data_.emplace_back(std::forward<Fn>(fn));
        }

        void operator()(const assert_data& adata) const noexcept override;
    };

    struct parse_assert_data_impl
    {
        wstring operator()(const assert_data& adata) const;
    };

    export constexpr parse_assert_data_impl parse_assert_data;
} // namespace fd
