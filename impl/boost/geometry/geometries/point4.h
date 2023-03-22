#pragma once

#include <boost/geometry/geometries/point.hpp>

template <typename CoordinateType, typename CoordinateSystem>
class boost::geometry::model::point<CoordinateType, 4, CoordinateSystem>
{
    enum
    {
        cs_check = sizeof(CoordinateSystem)
    };

    // ReSharper disable once CppInconsistentNaming
    CoordinateType m_values[4];

  public:
    constexpr point() = default;

    constexpr point(
        CoordinateType const &v0,
        CoordinateType const &v1,
        CoordinateType const &v2,
        CoordinateType const &v3)
        : m_values(v0, v1, v2, v3)
    {
    }

    template <std::size_t K>
    constexpr CoordinateType const &get() const
    {
        return m_values[K];
    }

    template <std::size_t K>
    void set(CoordinateType const &value)
    {
        m_values[K] = value;
    }
}; // namespace boost::geometry::model
