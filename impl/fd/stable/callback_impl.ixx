module;

#include <limits>
#include <type_traits>

export module fd.callback.impl;
export import fd.callback;
import fd.static_vector;

template <size_t ExtraBuffSize, typename... Args>
class callback_ex : virtual public fd::abstract_callback<Args...>
{
    using _Base = fd::abstract_callback<Args...>;

  public:
    using typename _Base::callback_type;
    using storage_type = fd::static_vector<callback_type, ExtraBuffSize>;

  private:
    storage_type storage_;

  public:
    void append(callback_type&& callback) final
    {
        storage_.push_back(std::move(callback));
    }

    void operator()(Args... args) const override
    {
        for (auto& fn : storage_)
            fd::invoke((callback_type&)fn, args...);
    }

    bool empty() const final
    {
        return storage_.empty();
    }

    size_t size() const final
    {
        return storage_.size();
    }
};

template <typename... Args>
using callback = callback_ex<0, Args...>;

template <size_t ExtraBuffSize, class C>
class callback_ex_custom : virtual public fd::abstract_callback_custom<C>
{
    using _Base = fd::abstract_callback_custom<C>;

  public:
    using typename _Base::callback_type;
    using storage_type = fd::static_vector<callback_type, ExtraBuffSize>;

  protected:
    storage_type storage_;

  public:
    void append(callback_type&& callback) override
    {
        storage_.push_back(std::move(callback));
    }

    // provide invoke manually

    bool empty() const final
    {
        return storage_.empty();
    }

    size_t size() const final
    {
        return storage_.size();
    }
};

template <class C>
using callback_custom = callback_ex_custom<0, C>;

export namespace fd
{

    using ::callback;
    using ::callback_custom;
    using ::callback_ex;
    using ::callback_ex_custom;
} // namespace fd
