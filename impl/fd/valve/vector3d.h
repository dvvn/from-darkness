#pragma once

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace fd::valve
{
struct vector3d : boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian>
{
};
}