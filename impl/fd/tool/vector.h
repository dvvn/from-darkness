#pragma once
#include "core.h"

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

#include <vector>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(vector, std::vector<T>);
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(small_vector, boost::container::small_vector<T, BufferSize>);
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(static_vector, boost::container::static_vector<T, BufferSize>);
} // namespace fd

// ReSharper disable CppInconsistentNaming
#ifdef _MSC_VER
_STD_BEGIN

template <typename T, bool C>
T _Get_unwrapped(boost::container::vec_iterator<T, C> it)
{
    return it.get_ptr();
}

_STD_END
#else

#endif
// ReSharper restore CppInconsistentNaming