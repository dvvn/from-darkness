#pragma once

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace fd::valve
{
struct vector2d : boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>
{
};
}