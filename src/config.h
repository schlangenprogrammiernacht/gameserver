/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*!
 * \file
 *
 * \brief Game constants configuration file.
 * \details
 * This file defines global, static constants used by different parts of the
 * game in a common place.
 */
#pragma once

#include <chrono>

#include "types.h"

namespace config {

	// Field size
	static const real_t     FIELD_SIZE_X            = 1920*1.5;
	static const real_t     FIELD_SIZE_Y            = 1080*1.5;

	// Spatial Map size
	static constexpr const size_t SPATIAL_MAP_TILES_X = 128;
	static constexpr const size_t SPATIAL_MAP_TILES_Y = 128;
	static constexpr const size_t SPATIAL_MAP_RESERVE_COUNT = 100;

	// Items of static food on field
	static const std::size_t FIELD_STATIC_FOOD       = 2000;

	// Steps that a snake moves while boosting (normal speed = 1 step)
	static const std::size_t SNAKE_BOOST_STEPS       = 2;

	// Distance per normal movement step
	static const real_t     SNAKE_DISTANCE_PER_STEP = 1.0;

	// Inner turn radius for a snake.
	// When the snake makes the smallest possible turn, it will surround a circle
	// with this value times the segment radius.
	static const real_t     SNAKE_TURN_RADIUS_FACTOR = 0.5;

	static const real_t     SNAKE_PULL_FACTOR        = 0.10;

	// Factor of snake mass to convert to food when snake dies
	static const real_t     SNAKE_CONVERSION_FACTOR  = 0.90;

	// segment distance = (mass * factor)^exponent
	static const real_t     SNAKE_SEGMENT_DISTANCE_FACTOR   = 0.2;
	static const real_t     SNAKE_SEGMENT_DISTANCE_EXPONENT = 0.3;

	// Distance multiplier for the Snake’s consume range. This is multiplied with
	// the Snake’s segment radius.
	static const real_t     SNAKE_CONSUME_RANGE     = 2.0;

	// Part of Snake's mass to drop in every frame the boost is on.
	static const real_t     SNAKE_BOOST_LOSS_FACTOR = 1e-3;

	// Snakes continuously loose mass. This parameter adjust the part of mass
	// lost every movement (==frame). The mass is not dropped, but simply lost.
	// 1e-5 means that a mass 100k-Snake has to eat food of value 1 on average every
	// frame to keep its mass.
	static const real_t     SNAKE_SURVIVAL_LOSS_FACTOR = 1e-5;

	// Mass below which Snake dies through starvation
	static const real_t     SNAKE_SELF_KILL_MASS_THESHOLD = 1.0;

	// Food particle size log-normal distribution parameters
	static const real_t     FOOD_SIZE_MEAN          = 3.5;
	static const real_t     FOOD_SIZE_STDDEV        = 2.0;

	// Food decay value per frame
	static const real_t     FOOD_DECAY_STEP         = 0.001;

	// Minimum mass ratio required that a bot can kill another bot. This prevents
	// that very small/newly spawned snakes can kill large snakes.
	// Example: 0.01 means that the killer has to have at least 1% of the victims
	// mass to be successful.
	static const real_t     KILLER_MIN_MASS_RATIO   = 0.001;

	// Lua memory pool configuration
	static const std::size_t LUA_MEM_POOL_SIZE_BYTES       = 25 * 1024*1024;
	static const std::size_t LUA_MEM_POOL_BLOCK_SIZE_BYTES = 256;

	// Log rate limiting for bots
	static constexpr const real_t LOG_CREDITS_PER_FRAME = 0.2;
	static constexpr const real_t LOG_INITIAL_CREDITS = 10;
	static constexpr const real_t LOG_MAX_CREDITS = 50;
	static constexpr const size_t LOG_MAX_MESSAGE_SIZE = 200;

	// maximum number of colors a bot can have
	static constexpr const size_t MAX_COLORS = 100;

	// bot IPC directory location
	static const char *BOT_IPC_DIRECTORY = "/mnt/spn_shm/";

	// script for launching new bots
	static const char *BOT_LAUNCHER_SCRIPT = "docker4bots/2_run_spn_cpp_bot.sh";

	// Timeout configuration (all times in seconds)
	static const real_t BOT_CONNECT_TIMEOUT = 10.000;
	static const real_t BOT_INIT_TIMEOUT   = 0.050;
	static const real_t BOT_STEP_TIMEOUT   = 0.010;

	// Maximum number of step() errors in a row before the bot is killed
	static const uint32_t BOT_MAX_STEP_ERRORS = 10;
}
