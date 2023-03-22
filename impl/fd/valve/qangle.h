#pragma once

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace fd::valve
{
struct qangle : boost::geometry::model::point<float, 3, boost::geometry::cs::spherical<boost::geometry::degree>>
{
    using point::point;
};
}