#pragma once

#include <cstdint>

// the following suppresses the only warning Eigen3 produces in -pedantic mode
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#include <Eigen/Geometry>
#pragma GCC diagnostic pop

typedef float         float_t;
typedef std::uint64_t guid_t;

typedef Eigen::Matrix<float_t, 2, 1> Vector2D;
