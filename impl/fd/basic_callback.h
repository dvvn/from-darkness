#pragma once

#include "core.h"

namespace fd
{
class callback_stop_token : public noncopyable
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

struct basic_callback_info
{
    virtual bool have_stop_token() const = 0;
};

template <typename Ret>
struct basic_callback : basic_callback_info
{
    using return_type = Ret;

    virtual return_type invoke(void *result, callback_stop_token *stop_token) = 0;
    virtual return_type invoke(void *result)                                  = 0;

    basic_callback *base()
    {
        return this;
    }
};

template <typename Ret, typename T>
struct basic_callback_proxy : basic_callback<Ret>
{
    virtual Ret invoke(T result, callback_stop_token *stop_token) = 0;
    virtual Ret invoke(T result)                                  = 0;

  private:
    Ret invoke(void *result, callback_stop_token *stop_token) final
    {
        return this->invoke(reinterpret_cast<T>(result), stop_token);
    }

    Ret invoke(void *result) final
    {
        return this->invoke(reinterpret_cast<T>(result));
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