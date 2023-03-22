#pragma once

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point4.h>

namespace fd::valve
{
struct vector4d : boost::geometry::model::point<float, 4, boost::geometry::cs::cartesian>
{
};
}