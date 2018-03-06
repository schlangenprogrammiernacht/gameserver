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

	// GlobalView grid unit
	static const float_t     GLOBALVIEW_GRID_UNIT    = 10.0;

	// Field size
	static const float_t     FIELD_SIZE_X            = 1024;
	static const float_t     FIELD_SIZE_Y            = 1024;

	// Items of static food on field
	static const std::size_t FIELD_STATIC_FOOD       = 5000;

	// Radius around the head of a snake in which food is eaten
	static const float_t     SNAKE_EATING_RADIUS     = 2.0;

	// Steps that a snake moves while boosting (normal speed = 1 step)
	static const std::size_t SNAKE_BOOST_STEPS       = 3;

	// Distance per normal movement step
	static const float_t     SNAKE_DISTANCE_PER_STEP = 1.0;

	static const float_t     SNAKE_PULL_FACTOR        = 0.10;

	// segment distance = (mass * factor)^exponent
	static const float_t     SNAKE_SEGMENT_DISTANCE_FACTOR   = 0.2;
	static const float_t     SNAKE_SEGMENT_DISTANCE_EXPONENT = 0.3;

	// Distance multiplier for the Snake’s consume range. This is multiplied with
	// the Snake’s segment radius.
	static const float_t     SNAKE_CONSUME_RANGE     = 1.0;

	// Food particle size log-normal distribution parameters
	static const float_t     FOOD_SIZE_MEAN          = 3.5;
	static const float_t     FOOD_SIZE_STDDEV        = 2.0;

	// Food decay value per frame
	static const float_t     FOOD_DECAY_STEP         = 0.001;
}
