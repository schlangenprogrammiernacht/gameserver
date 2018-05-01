#pragma once

#include <cstdint>

#ifdef __clang__
#include <Eigen/Geometry>
#else
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#include <Eigen/Geometry>
#pragma GCC diagnostic pop
#endif

typedef float real_t;
typedef std::uint64_t guid_t;

typedef Eigen::Matrix<real_t, 2, 1> Vector2D;
