module;

#include <utility>

export module fd.assert.impl;
export import fd.assert;
export import fd.string;
import fd.callback;
import fd.mutex;

using namespace fd;

class default_assert_handler : public assert_handler_t
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

    void operator()(const assert_data& adata) const override;
};

wstring parse_assert_message(const assert_data& adata);

export namespace fd
{
    using ::default_assert_handler;
    using ::parse_assert_message;
} // namespace fd
