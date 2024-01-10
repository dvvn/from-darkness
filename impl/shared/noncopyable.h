#pragma once

#include "type_traits/type_identity.h"

#include <boost/core/noncopyable.hpp>

#include <concepts>

namespace fd
{
using
#ifdef __RESHARPER__
    noncopyable =
#endif
        boost::noncopyable;

#if 0
namespace detail
{
template <class T, bool Copyable = std::copyable<T>>
struct noncopyable_wrapper_impl;

template <class T>
struct noncopyable_wrapper_impl<T, true> : T, noncopyable
{
    using type = noncopyable_wrapper_impl;
    using T::T;
};

template <class T>
struct noncopyable_wrapper_impl<T, false> : type_identity<T>
{
};
} // namespace detail

template <class T>
using noncopyable_wrapper = typename detail::noncopyable_wrapper_impl<T>::type;
#endif
} // namespace fd