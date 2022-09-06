module;

#include <limits>
#include <veque.hpp>

#include <deque>

export module fd.callback.impl;
export import fd.callback;

using namespace fd;

template <typename Ret, typename... Args>
class basic_callback : virtual public abstract_callback<Ret, Args...>
{
    using _Base = abstract_callback<Ret, Args...>;

  public:
    using typename _Base::callback_type;

    using storage_type =
#ifdef VEQUE_HEADER_GUARD
        veque::veque
#else
        std::deque
#endif
        <callback_type>;

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
            invoke(fn, args...);
    }
};

template <typename... Args>
struct callback<bool, Args...> : basic_callback<bool, Args...>
{
    void operator()(Args... args) override
    {
        for (auto& fn : this->storage_)
        {
            if (!invoke(fn, args...))
                break;
        }
    }
};

template <class C>
class callback_custom : virtual public abstract_callback_custom<C>
{
    using _Base = abstract_callback_custom<C>;

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
