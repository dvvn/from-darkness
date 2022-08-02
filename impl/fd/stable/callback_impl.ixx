module;

//#include <veque.hpp>

#include <deque>
#include <limits>
#include <type_traits>

export module fd.callback.impl;
export import fd.callback;

template <typename Ret, typename... Args>
class basic_callback : virtual public fd::abstract_callback<Ret, Args...>
{
    using _Base = fd::abstract_callback<Ret, Args...>;

  public:
    using typename _Base::callback_type;
    using storage_type = std::deque<callback_type>;

  protected:
    storage_type storage_;

  public:
    void push_back(callback_type&& callback) override
    {
        storage_.push_back(std::move(callback));
    }

    void push_front(callback_type&& callback) override
    {
        storage_.push_front(std::move(callback));
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

template <typename Ret, typename... Args>
struct callback : basic_callback<Ret, Args...>
{
};

template <typename... Args>
struct callback<void, Args...> : basic_callback<void, Args...>
{
    void operator()(Args... args) override
    {
        for (auto& fn : this->storage_)
            fd::invoke(fn, args...);
    }
};

template <typename... Args>
struct callback<bool, Args...> : basic_callback<bool, Args...>
{
    void operator()(Args... args) override
    {
        for (auto& fn : this->storage_)
        {
            if (!fd::invoke(fn, args...))
                break;
        }
    }
};

template <class C>
class callback_custom : virtual public fd::abstract_callback_custom<C>
{
    using _Base = fd::abstract_callback_custom<C>;

  public:
    using callback_type = typename _Base::callback_type;
    using storage_type  = std::deque<callback_type>;

  protected:
    storage_type storage_;

  public:
    void push_back(callback_type&& callback) override
    {
        storage_.push_back(std::move(callback));
    }

    void push_front(callback_type&& callback) override
    {
        storage_.push_front(std::move(callback));
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

export namespace fd
{
    using ::callback;
    using ::callback_custom;
} // namespace fd
