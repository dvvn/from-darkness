#pragma once

#include "type_traits/type_identity.h"

#include <boost/core/noncopyable.hpp>

namespace fd
{
using
#ifdef __RESHARPER__
    noncopyable =
#endif
        boost::noncopyable;

template <class T, bool Copyable = std::copyable<T>>
struct noncopyable_proxy;

template <class T>
struct noncopyable_proxy<T, true> : T, noncopyable, type_identity<noncopyable_proxy<T, false>>
{
};

template <class T>
struct noncopyable_proxy<T, false> : type_identity<T>
{
};

template <class T>
using noncopyable_proxy_t = typename noncopyable_proxy<T>::type;
} // namespace fd