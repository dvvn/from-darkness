#pragma once

#include <boost/lambda2.hpp>

#include <functional>

namespace fd
{
namespace placeholders = boost::lambda2;

using std::bind;
using std::bind_back;
using std::bind_front;
}