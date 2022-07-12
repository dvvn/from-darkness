module;

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <variant>

export module fd.callback.impl;
export import fd.callback;
import fd.static_vector;

template <size_t ExtraBuffSize, typename... Args>
class callback_ex : public fd::abstract_callback<Args...>
{
    using _Base = fd::abstract_callback<Args...>;

  public:
    using typename _Base::callback_type;
    using storage_type = fd::static_vector<callback_type, ExtraBuffSize>;

  private:
    storage_type data_;

  public:
    void append(callback_type&& callback) override
    {
        data_.push_back(std::move(callback));
    }

    void invoke(Args... args) const override
    {
        for (auto& fn : data_)
            std::invoke(fn, args...);
    }

    bool empty() const override
    {
        return data_.empty();
    }
};

template <typename... Args>
using callback = callback_ex<0, Args...>;

export namespace fd
{
    using ::callback;
    using ::callback_ex;
} // namespace fd
