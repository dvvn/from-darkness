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
class noncopyable_assginable : boost::noncopyable_::base_token
{
  protected:
    ~noncopyable_assginable()                             = default;
    constexpr noncopyable_assginable()                    = default;
    noncopyable_assginable(noncopyable_assginable const&) = delete;
};

class noncopyable_constructible : boost::noncopyable_::base_token
{
  protected:
    ~noncopyable_constructible()          = default;
    constexpr noncopyable_constructible() = default;

    noncopyable_constructible& operator=(noncopyable_constructible const&) = delete;
};
#endif
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