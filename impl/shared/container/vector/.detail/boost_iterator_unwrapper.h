#pragma once

//#include ".detail/wrapper.h"

#include <boost/container/vector.hpp>

//namespace fd
//{
//template <typename T>
//FD_WRAP_TOOL(boost_vector, boost::container::vector<T>);
//}

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