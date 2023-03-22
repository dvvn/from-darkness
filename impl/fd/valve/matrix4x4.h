#pragma once

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point4.h>

#include <array>

namespace fd::valve
{
struct matrix4x4 : boost::geometry::model::point<std::array<float, 4>, 4, boost::geometry::cs::cartesian>
{
    using point::point;
};
}