module;

#include <utility>

export module fd.assert.impl;
export import fd.assert;
export import fd.string;
import fd.callback;
import fd.mutex;

using namespace fd;

class default_assert_handler : public basic_assert_handler
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

struct assert_data_parsed : wstring
{
    assert_data_parsed(const assert_data& adata);
};

export namespace fd
{
    using ::assert_data_parsed;
    using ::default_assert_handler;
} // namespace fd
