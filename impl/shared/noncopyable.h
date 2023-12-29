#pragma once

#include <boost/core/noncopyable.hpp>

namespace fd
{
using
#ifdef __RESHARPER__
    noncopyable =
#endif
        boost::noncopyable;
} // namespace fd