/*!
 * \file
 *
 * \brief Game constants configuration file.
 * \details
 * This file defines global, static constants used by different parts of the
 * game in a common place.
 */
#pragma once

#include "types.h"

namespace config {

	// Field size
	static const real_t     FIELD_SIZE_X            = 1920;
	static const real_t     FIELD_SIZE_Y            = 1080;

	// Spatial Map size
	static constexpr const size_t SPATIAL_MAP_TILES_X = 128;
	static constexpr const size_t SPATIAL_MAP_TILES_Y = 128;
	static constexpr const size_t SPATIAL_MAP_RESERVE_COUNT = 10;

	// Items of static food on field
	static const std::size_t FIELD_STATIC_FOOD       = 1000;

	// Steps that a snake moves while boosting (normal speed = 1 step)
	static const std::size_t SNAKE_BOOST_STEPS       = 3;

	// Distance per normal movement step
	static const real_t     SNAKE_DISTANCE_PER_STEP = 1.0;

	static const real_t     SNAKE_PULL_FACTOR        = 0.10;

	// Factor of snake mass to convert to food when snake dies
	static const real_t     SNAKE_CONVERSION_FACTOR  = 0.50;

	// segment distance = (mass * factor)^exponent
	static const real_t     SNAKE_SEGMENT_DISTANCE_FACTOR   = 0.2;
	static const real_t     SNAKE_SEGMENT_DISTANCE_EXPONENT = 0.3;

	// Distance multiplier for the Snake’s consume range. This is multiplied with
	// the Snake’s segment radius.
	static const real_t     SNAKE_CONSUME_RANGE     = 2.0;

	// Food particle size log-normal distribution parameters
	static const real_t     FOOD_SIZE_MEAN          = 3.5;
	static const real_t     FOOD_SIZE_STDDEV        = 2.0;

	// Food decay value per frame
	static const real_t     FOOD_DECAY_STEP         = 0.001;
}
