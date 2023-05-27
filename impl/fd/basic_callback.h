#pragma once

#include <boost/core/noncopyable.hpp>

namespace fd
{
class callback_stop_token : public boost::noncopyable
{
    bool value_ = false;

  public:
    void stop()
    {
        value_ = true;
    }

    bool stop_requested() const
    {
        return value_;
    }
};

template <typename Ret>
struct basic_callback
{
    virtual Ret invoke(void *result, callback_stop_token *stop_token) = 0;

    basic_callback *base()
    {
        return this;
    }
};

template <typename Ret, typename T>
struct basic_callback_proxy : basic_callback<Ret>
{
    virtual Ret invoke(T result, callback_stop_token *stop_token) = 0;

  private:
    Ret invoke(void *result, callback_stop_token *stop_token) final
    {
        return this->invoke(reinterpret_cast<T>(result), stop_token);
    }

    /*void invoke(void *result) final
    {
        this->invoke(reinterpret_cast<T>(result));
    }*/
};

template <typename Ret>
struct basic_callback_proxy<Ret, void *> : basic_callback<Ret>
{
};
}