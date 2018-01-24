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

	// Radius around the head of a snake in which food is eaten
	static const float_t     SNAKE_EATING_RADIUS     = 2.0;

	// Steps that a snake moves while boosting (normal speed = 1 step)
	static const std::size_t SNAKE_BOOST_STEPS       = 3;

	// Distance per normal movement step
	static const float_t     SNAKE_DISTANCE_PER_STEP = 1.0;

}
